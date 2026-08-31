#!/usr/bin/env python3
"""
Zephyr whitebox report generator.

从评测结果 results.jsonl + trajectory 日志,把失败任务分类到统一 L2 taxonomy,
派生 mechanism(幻觉 / 理解偏差 / 假失败),剔除假失败,
产出与 QSemOS / RIOT 同 schema 的 whitebox_report.json,保证跨 track 可比。

对齐约定(见《给Yujie整改清单_PR3.md》第四节):
  - L1 = runner 的 error_category(compile_error / crash / test_failure / false_failure_detection)
  - L2 = 细粒度子类(统一 taxonomy,经 L2_CONSOLIDATED 合并)
  - mechanism = L2_TO_MECH 派生(幻觉 / 理解偏差 / false_failure)
  - 指令遵循失败(illegal_changes 非空)→ 不进入 whitebox(在 results.jsonl 单独统计)
  - is_false_failure(harness bug)→ 剔除,不计入失败

用法:
  python whitebox.py [--results results/results.jsonl]
                     [--trajectory trajectory]
                     [--out results/whitebox_report.json]
"""
import argparse
import json
import re
from collections import Counter
from pathlib import Path

BASE = Path(__file__).resolve().parent

# ---- 统一 L2 taxonomy(与 qsem/riot 参考实现一致) ----
L2_CONSOLIDATED = {
    # -- hallucination --
    'undefined_reference': 'undefined_reference',
    'hallucinated_symbol': 'undefined_reference',
    'hallucinated_api': 'hallucinated_api',
    'hallucinated_api_and_wrong_architecture': 'hallucinated_api',
    'implicit_declaration': 'hallucinated_api',
    # -- missing/excess guard --
    'missing_conditional_guard': 'missing_conditional_guard',
    'missing_null_check': 'missing_conditional_guard',
    'missing_corner_cases': 'missing_conditional_guard',
    'missing_synchronization': 'missing_conditional_guard',
    'extra_conditional_guard': 'extra_conditional_guard',
    # -- wrong calculation --
    'wrong_calculation': 'wrong_calculation',
    'wrong_operator': 'wrong_calculation',
    'bit_manipulation_error': 'wrong_calculation',
    'loop_logic_error': 'wrong_calculation',
    'accumulator_precision': 'wrong_calculation',
    # -- logic deviation --
    'logic_deviation': 'logic_deviation',
    'architecture_error': 'logic_deviation',
    'side_effect_error': 'logic_deviation',
    'validation_logic_inverted': 'logic_deviation',
    'wrong_error_semantics': 'logic_deviation',
    'wrong_error_handling': 'logic_deviation',
    'data_structure_order': 'logic_deviation',
    'data_structure_misuse': 'logic_deviation',
    'control_flow_error': 'logic_deviation',
    'type_error': 'logic_deviation',
    'mixed': 'logic_deviation',
    'algorithm_incomplete': 'logic_deviation',
    'semantic_misunderstanding': 'logic_deviation',
    'mixed_guard_and_loop': 'logic_deviation',
    'missing_include': 'logic_deviation',
    'missing_context_switch': 'logic_deviation',
    'hardware_knowledge_gap': 'logic_deviation',
    'incomplete_implementation': 'logic_deviation',
    # -- semantic drift --
    'semantic_drift': 'semantic_drift',
    'wrong_return_value': 'semantic_drift',
    # -- missing error path --
    'missing_error_path': 'missing_error_path',
    'missing_api_call': 'missing_error_path',
    # -- RIOT-specific --
    'werror_style': 'werror_style',
    'test_not_executed': 'test_not_executed',
    # -- special --
    'needs_manual_inspection': 'needs_manual_inspection',
    'unclassified': 'needs_manual_inspection',
    'unknown_crash': 'needs_manual_inspection',
    'harness_bug_crash_regex': 'harness_bug_crash_regex',
    'empty_stub': 'logic_deviation',
    'bitmask_error': 'wrong_calculation',
}

HALLUCINATION_L2 = {'undefined_reference', 'hallucinated_api'}

# ---- Zephyr 模块 L1 划分(整改清单四节:OS 模块维度) ----
# L1 = source_path 顶层目录;include/ 下的头文件内联函数按其子系统归并。
# include/zephyr/<sub>/... 映射到 <sub>(与 subsys/ 对齐),其余保留顶层目录。
ZEPHYR_MODULE_L1 = {
    "kernel", "subsys", "lib", "drivers", "arch",
    "include", "modules", "samples", "bootloader", "other",
}


def classify_module(source_path: str) -> str:
    """给出 Zephyr 模块 L1 划分。

    - 顶层目录(除 include)直接作为 L1;
    - include/zephyr/<sub>/... → 映射到 <sub>(头文件内联函数归到其子系统);
    - 其它无法归类 → 'other'。
    """
    if not source_path:
        return "other"
    parts = source_path.split("/")
    top = parts[0]
    if top == "include":
        # include/zephyr/<sub>/... → <sub>
        for p in parts[1:]:
            if p == "zephyr":
                continue
            return p
        return "include"
    return top if top in ZEPHYR_MODULE_L1 else "other"


def read_trajectory(traj_dir: Path, tid: str) -> str:
    """读取任务轨迹日志,重点提取 Harness Verify 段(build/test 输出)。"""
    p = traj_dir / f"task_{tid}.log"
    if not p.exists():
        return ""
    text = p.read_text(errors="replace")
    # 只要 Harness Verify 段(构建/测试错误都在这)
    if "=== Harness Verify ===" in text:
        text = text.split("=== Harness Verify ===", 1)[1]
    return text


def infer_L2(result: dict, traj_text: str) -> str:
    """从 error_category + 轨迹文本推断统一 L2 子类。"""
    cat = result.get("error_category", "")
    low = (traj_text or "").lower()

    if cat == "compile_error":
        if "undefined reference" in low:
            return "undefined_reference"        # 幻觉:调用了不存在的符号
        if "implicit declaration" in low:
            return "implicit_declaration"       # 幻觉:隐式声明
        if "undeclared" in low:
            return "hallucinated_api"           # 幻觉:编造了不存在的 API
        if ("unterminated" in low or "#else" in low or "#endif" in low
                or "preprocessor" in low):
            return "missing_conditional_guard"  # 理解偏差:破坏条件编译
        if "werror" in low or "-werror" in low:
            return "werror_style"               # 警告被当作错误
        return "needs_manual_inspection"

    if cat == "crash":
        if "segmentation fault" in low or "segfault" in low:
            return "missing_null_check"         # 理解偏差:漏空指针检查
        if "panic" in low or "aborted" in low:
            return "missing_null_check"
        return "needs_manual_inspection"        # 未知崩溃,需人工

    if cat == "test_failure":
        # 编译过但测试失败——无法仅从报错文本判断机理,归入需人工
        return "needs_manual_inspection"

    if cat == "test_not_executed":
        return "test_not_executed"

    return "needs_manual_inspection"


def is_false_failure(result: dict, traj_text: str) -> bool:
    """harness 级误判:剔除,不计入失败。"""
    cat = result.get("error_category", "")
    if cat in ("timeout", "watchdog", "api_error", "exception"):
        return True
    low = (traj_text or "").lower()
    if "utf-8" in low and ("decode" in low or "invalid" in low or "codec" in low):
        return True
    return False


def build_report(results_path, traj_dir, out_path):
    results = []
    for line in open(results_path, encoding="utf-8"):
        line = line.strip()
        if line:
            results.append(json.loads(line))

    if not results:
        print(f"WARNING: {results_path} 为空——请先运行评测( python runner.py --batch N )")
        return

    wb = []
    n_illegal = 0        # 指令遵循失败(在 results.jsonl,不进 whitebox)
    for r in results:
        if r.get("passed"):
            continue
        tid = str(r["task_id"])
        # 指令遵循失败:illegal_changes 非空 → 不进入 whitebox
        if r.get("illegal_changes"):
            n_illegal += 1
            continue

        traj = read_trajectory(traj_dir, tid)
        ff = is_false_failure(r, traj)
        l1 = "false_failure_detection" if ff else (r.get("error_category") or "unknown")
        raw_l2 = "harness_bug_*" if ff else infer_L2(r, traj)
        l3_detail = raw_l2               # 合并前的细粒度根因(如 missing_null_check)
        l2 = L2_CONSOLIDATED.get(raw_l2, raw_l2)

        # mechanism 派生(与参考实现一致)
        if ff:
            mechanism = "false_failure"
        elif l2 in HALLUCINATION_L2:
            mechanism = "hallucination"
        else:
            mechanism = "misunderstanding"

        # module: Zephyr 模块 L1 划分(整改清单四节)
        src = r.get("snapshot", {}).get("task", {}).get("source_path", "")
        module = classify_module(src)

        wb.append({
            "task_id": tid,
            "module": module,
            "classification": {"L1": l1, "L2": l2},
            "l3_detail": l3_detail,
            "mechanism": mechanism,
            "is_false_failure": ff,
            "actual_pass": False,
        })

    # 写输出
    out_path = Path(out_path)
    out_path.parent.mkdir(parents=True, exist_ok=True)
    with open(out_path, "w", encoding="utf-8") as f:
        json.dump(wb, f, ensure_ascii=False, indent=2)

    # 统计
    real = [w for w in wb if not w["is_false_failure"]]
    ff = [w for w in wb if w["is_false_failure"]]
    l1c = Counter(w["classification"]["L1"] for w in real)
    l2c = Counter(w["classification"]["L2"] for w in real)
    mechc = Counter(w["mechanism"] for w in real)
    modc = Counter(w["module"] for w in real)

    print(f"=== whitebox report ({len(wb)} entries, {len(real)} real, {len(ff)} false-failure) ===")
    print(f"  指令遵循失败(illegal_changes, 不进 whitebox): {n_illegal}")
    print(f"  mechanism: {dict(mechc)}")
    print(f"  L1: {dict(l1c)}")
    print(f"  L2 ({len(l2c)} types):")
    for k, v in l2c.most_common():
        print(f"    {k}: {v}")
    print(f"  module: {dict(modc)}")
    print(f"\nResults saved to {out_path}")


def main():
    parser = argparse.ArgumentParser(description="Zephyr whitebox report generator")
    parser.add_argument("--results", default=str(BASE / "results" / "results.jsonl"))
    parser.add_argument("--trajectory", default=str(BASE / "trajectory"))
    parser.add_argument("--out", default=str(BASE / "results" / "whitebox_report.json"))
    args = parser.parse_args()

    results_path = Path(args.results)
    if not results_path.exists():
        print(f"ERROR: 找不到结果文件 {results_path}")
        print("请先运行评测: cd zephyr-claude && python runner.py --batch N")
        return

    build_report(results_path, Path(args.trajectory), Path(args.out))


if __name__ == "__main__":
    main()
