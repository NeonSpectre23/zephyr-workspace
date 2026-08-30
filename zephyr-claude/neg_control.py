#!/usr/bin/env python3
"""负控制检查：对每个任务注入"空实现"再跑测试，找出测试并未真正验证目标函数的弱任务。

原理：
  1. 先注入 oracle（黄金答案）跑测试 —— 正常情况应 PASS（数据集正确）。
  2. 再注入一个能编译的空实现（stub）跑同一个测试。
  3. 若 oracle PASS 而空实现也 PASS → 说明该测试没有实际验证 sut_function
     （函数没被调用 / 没被断言 / 文件根本没编译进该 build）→ 标记为 WEAK（弱任务/假验证）。
     这类任务在真实评测里对任何模型都"恒通过"，会虚增通过率，应修或弃。

判定：
  ORACLE_SKIP      函数定义找不到（数据集 source_path/sut_function 缺陷）
  ORACLE_FAIL      oracle 注入后测试不过（run_command/unit_test 缺陷）
  NEG_COMPILE_FAIL 空实现没能编译（多半是脚本 stub 生成问题，需人工看）
  GOOD             oracle PASS 且空实现 FAIL → 测试真验证了函数
  WEAK             oracle PASS 且空实现也 PASS → 测试没验证函数 ★目标发现

用法：
  python neg_control.py --batch 5
  python neg_control.py --task-id 3
  python neg_control.py --batch 100 --resume          # 断点续跑，自动读 neg_control.log
  python neg_control.py --resume neg_control.log     # 或显式指定日志文件
输出：neg_control_results.json + 进度日志（默认 neg_control.log）
      + neg_control_logs/（每个任务的 oracle/stub 完整 build 日志，便于诊断）
"""

import argparse
import json
import re
import subprocess
from collections import Counter
from pathlib import Path

from inject_test import (
    BENCH_DIR,
    ZEPHYR_DIR,
    load_tasks,
    git_reset,
    run_test,
    inject_oracle,
    _find_function,
    _char_offset,
)

COMPILE_FAIL_MARKERS = ("error:", "build failed", "ninja: error",
                        "undefined reference", "ld returned")


def _full_return_type(source: str, fn_node) -> str:
    """提取完整返回类型文本（含指针星号）。

    tree-sitter 的 type 字段不含声明符里的 '*'——
    如 `struct foo *bar(...)` 的 type 只是 `struct foo`，星号在 declarator 里。
    沿 declarator 链数 pointer_declarator 层数，补上星号。
    """
    ret = fn_node.child_by_field_name("type")
    ret_text = ""
    if ret is not None:
        ret_text = source[_char_offset(source, ret.start_byte)
                          :_char_offset(source, ret.end_byte)].strip()
    stars = ""
    node = fn_node.child_by_field_name("declarator")
    while node is not None and node.type == "pointer_declarator":
        stars += "*"
        node = node.child_by_field_name("declarator")
    return (ret_text + " " + stars).strip() or "int"


def _stub_return(ret_text: str) -> str:
    """按返回类型生成空实现的 return 语句。

    目标是返回一个"显然不是正确实现"的值，让真正验证该函数的测试失败：
      void             -> return;
      指针             -> return NULL;
      浮点             -> return 0.0;
      无符号数值/尺寸  -> return 0;   （如 size_t 的 bin2hex，0 是错误路径）
      其它（有符号等） -> return -1;  （让 err == 0 的成功断言失败）

    旧的实现一律 `return (类型){0};`，对"返回 0 = 成功"的 API（如 bt_enable、
    bt_le_scan_start）空实现返回 0 恰好通过成功断言 → 误判 WEAK。
    """
    clean = re.sub(r"\b(const|volatile|restrict)\b", "", ret_text)
    clean = re.sub(r"\b(struct|enum|union)\b", "", clean)
    clean = " ".join(clean.split())
    if clean == "void":
        return "\treturn;"
    if "*" in clean:
        return "\treturn NULL;"
    if re.match(r"^(float|double)", clean):
        return "\treturn 0.0;"
    # 显式无符号类型 → 0
    if re.match(r"^(unsigned|uint|u8_|u16_|u32_|u64_|size_t|_Bool|bool)", clean):
        return "\treturn 0;"
    # 显式有符号类型 → -1（让 err == 0 的成功断言失败）
    if re.match(r"^(signed|int|long|short|char|ssize_t|s8_|s16_|s32_|s64_)", clean):
        return "\treturn -1;"
    # 未知 typedef：可能是无符号，返回 0 最安全（避免 -Wsign-conversion 警告）
    return "\treturn 0;"


def build_stub_text(source: str, fn_name: str):
    """构造一个能编译的空实现，替换原函数定义；失败返回 None。

    保留函数签名（含 static/attributes），body 换成 _stub_return() 生成的值，
    保证 (a) 能编译 (b) 返回的不是正确实现，让真验证函数的测试失败。
    """
    node = _find_function(source, fn_name)
    if node is None:
        return None
    body_node = node.child_by_field_name("body")
    if body_node is None:
        return None
    a = _char_offset(source, node.start_byte)
    brace = _char_offset(source, body_node.start_byte)   # '{' 的位置
    end = _char_offset(source, node.end_byte)

    sig = source[a:brace]  # 签名 + 属性，到 '{' 之前（含前导空白）
    ret_text = _full_return_type(source, node)
    stub = "{\n%s\n}" % _stub_return(ret_text)

    # 必须保留函数之前的内容（#include、前置函数如 char2hex 等），
    # 否则注入 stub 后文件头被删，整个文件无法编译/链接。
    return source[:a] + sig + stub + "\n" + source[end:]


def inject_stub(task) -> bool:
    """用空实现替换目标函数；返回 False 表示找不到函数/无 body。"""
    src_path = ZEPHYR_DIR / task["source_path"]
    source = src_path.read_text(errors="replace")
    new_source = build_stub_text(source, task["sut_function"])
    if new_source is None:
        return False
    src_path.write_text(new_source)
    return True


def classify(stub_ok: bool, stub_fail_output: str) -> str:
    """oracle 已 PASS 的前提下，根据空实现一轮的结果判定。

    判定顺序：
      1. stub_ok              → WEAK（空实现也过，测试没验证函数）
      2. 测试真的跑了
         （出现 PROJECT EXECUTION SUCCESSFUL / FAILED）
         → GOOD（build 成功、测试失败 → 函数被真实验证）
      3. 测试没跑 + 有编译/链接错误标记
         → NEG_COMPILE_FAIL（build 阶段失败）

    注意：不能用 "error:" 单独当编译失败标记——ZTEST 断言失败也打印
    "error: (...)"，会把"空实现让测试失败"(实为 GOOD)误判成 NEG_COMPILE_FAIL。
    这里先判断测试是否执行，测试没跑时 "error:" 才只可能来自构建。
    """
    if stub_ok:
        return "WEAK"                    # 空实现也能过 → 测试没验证函数
    test_ran = ("PROJECT EXECUTION SUCCESSFUL" in stub_fail_output
                or "PROJECT EXECUTION FAILED" in stub_fail_output)
    if test_ran:
        return "GOOD"                    # build 成功、测试失败 → 函数被验证
    if any(m in stub_fail_output.lower() for m in COMPILE_FAIL_MARKERS):
        return "NEG_COMPILE_FAIL"        # 测试没跑 + 编译错误 → stub 没编译
    return "GOOD"                        # 无法确认是 build 失败 → 保守按测试失败


def main():
    parser = argparse.ArgumentParser(description="Negative-control check for dataset tasks")
    parser.add_argument("--task-id", type=str, help="只检查指定任务")
    parser.add_argument("--batch", type=int, help="只检查前 N 个任务")
    parser.add_argument("--resume", metavar="LOG", nargs="?", const=True,
                        help="断点续跑：跳过日志中已检查过的任务；省略 LOG 时用 --log 指定的文件")
    parser.add_argument("--log", metavar="FILE", default="neg_control.log",
                        help="进度日志文件（默认 neg_control.log）")
    parser.add_argument("--out", metavar="JSON", default="neg_control_results.json",
                        help="结果文件（默认 neg_control_results.json）")
    parser.add_argument("--recheck", metavar="VERDICT",
                        help="只重跑结果文件中指定判定的任务，如 NEG_COMPILE_FAIL")
    parser.add_argument("--results", metavar="FILE", default="neg_control_results.json",
                        help="--recheck 读取判定用的结果文件（默认 neg_control_results.json）")
    args = parser.parse_args()

    tasks = load_tasks()
    if args.task_id:
        tasks = [t for t in tasks if t["task_id"] == args.task_id]
    if args.batch:
        tasks = tasks[:args.batch]

    skip = set()
    if args.resume:
        resume_file = args.resume if isinstance(args.resume, str) else args.log
        try:
            for line in open(resume_file, encoding="utf-8", errors="replace"):
                m = re.match(r"task (\S+): (\S+)", line)
                if m:
                    skip.add(m.group(1))
        except OSError:
            print("WARNING: cannot read resume log: %s" % resume_file)
        tasks = [t for t in tasks if t["task_id"] not in skip]
        print("Resume: skipping %d already-checked tasks, %d remaining"
              % (len(skip), len(tasks)))

    # --recheck: 只重跑结果文件中指定判定的任务
    if args.recheck:
        verdict = args.recheck.upper()
        want = set()
        try:
            prev = json.load(open(args.results, encoding="utf-8"))
            for tid, r in prev.items():
                if str(r.get("verdict", "")).upper() == verdict:
                    want.add(str(tid))
        except OSError:
            print("WARNING: cannot read results file: %s" % args.results)
        tasks = [t for t in tasks if t["task_id"] in want]
        print("Recheck: %d tasks with verdict %s" % (len(tasks), verdict))

    results = {}
    total = len(tasks)
    logs_dir = Path(args.log).resolve().parent / "neg_control_logs"
    for i, task in enumerate(tasks):
        tid = task["task_id"]
        git_reset()
        if not inject_oracle(task):
            verdict = "ORACLE_SKIP"
        else:
            oracle_ok, _ = run_test(task,
                                    log_path=logs_dir / f"task_{tid}_oracle.log")
            if not oracle_ok:
                verdict = "ORACLE_FAIL"
            else:
                git_reset()
                if not inject_stub(task):
                    verdict = "ORACLE_SKIP"
                else:
                    try:
                        stub_ok, stub_tail = run_test(
                            task, log_path=logs_dir / f"task_{tid}_stub.log")
                    except subprocess.TimeoutExpired:
                        stub_ok, stub_tail = False, "[TIMEOUT]"
                    verdict = classify(stub_ok, stub_tail)

        results[tid] = {
            "task_id": tid,
            "sut_function": task["sut_function"],
            "source_path": task["source_path"],
            "unit_test": task.get("unit_test", ""),
            "run_command": task.get("run_command", ""),
            "verdict": verdict,
        }
        line = "[%d/%d] task %s: %s" % (i + 1, total, tid, verdict)
        print(line, flush=True)
        with open(args.log, "a", encoding="utf-8") as f:
            f.write("task %s: %s\n" % (tid, verdict))

    git_reset()
    with open(args.out, "w", encoding="utf-8") as f:
        json.dump(results, f, indent=2, ensure_ascii=False)

    counts = Counter(r["verdict"] for r in results.values())
    print("\n=== %d tasks checked ===" % len(results))
    for v in sorted(counts):
        print("  %-18s %5d" % (v, counts[v]))
    weak = [r for r in results.values() if r["verdict"] == "WEAK"]
    if weak:
        print("\nWEAK (test does not validate function):")
        for r in weak:
            print("  task %-4s %-28s %s  unit_test=%s"
                  % (r["task_id"], r["sut_function"], r["source_path"], r["unit_test"]))
    print("\nResults saved to %s" % args.out)


if __name__ == "__main__":
    main()
