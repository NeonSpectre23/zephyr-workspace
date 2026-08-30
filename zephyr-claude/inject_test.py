#!/usr/bin/env python3
"""注入 oracle → 编译 → 测试，用 git 管理源码状态。

用法:
  python3 inject_test.py --task-id 1
  python3 inject_test.py --batch 5
  python3 inject_test.py --batch 100            # 进度实时写入 inject_test.log
  python3 inject_test.py --batch 100 --resume   # 断点续跑，跳过已 PASS 的任务
"""

import argparse
import json
import os
import re
import subprocess
from pathlib import Path

from mask import _find_function, _char_offset

SCRIPT_DIR = Path(__file__).resolve().parent
BENCH_DIR = SCRIPT_DIR.parent / "zephyr-bench"
ZEPHYR_DIR = SCRIPT_DIR.parent / "zephyr"
SDK_DIR = Path(os.environ.get(
    "ZEPHYR_SDK_INSTALL_DIR",
    str(Path.home() / "zephyr-sdk-1.0.1"),
))


def load_tasks():
    tasks = []
    for fname in ["zephyr_tasks.c.jsonl", "zephyr_tasks.h.jsonl"]:
        fpath = BENCH_DIR / fname
        if fpath.exists():
            with open(fpath) as f:
                for line in f:
                    if line.strip():
                        tasks.append(json.loads(line))
    return tasks


def git_reset():
    subprocess.run(["git", "reset", "--hard", "HEAD"],
                   cwd=ZEPHYR_DIR, capture_output=True)
    subprocess.run(["git", "clean", "-fd", "-e", "zephyr-claude",
                    "-e", "zephyr-bench"],
                   cwd=ZEPHYR_DIR, capture_output=True)


def inject_oracle(task):
    """用 oracle 替换目标函数。"""
    src = ZEPHYR_DIR / task["source_path"]
    oracle = BENCH_DIR / task["oracle"]

    source = src.read_text(errors="replace")
    oracle_text = oracle.read_text(errors="replace").strip()

    fn_node = _find_function(source, task["sut_function"])
    if fn_node is None:
        return False

    a = _char_offset(source, fn_node.start_byte)
    b = _char_offset(source, fn_node.end_byte)
    src.write_text(source[:a] + oracle_text + "\n" + source[b:])
    return True


def run_test(task, log_path=None):
    """运行 west build + 测试。

    Args:
        task: 任务字典
        log_path: 可选，把完整 stdout/stderr 写入该文件（便于诊断失败原因）
    """
    cmd = task["run_command"]
    test_dir = ""
    if "&&" in cmd:
        parts = cmd.split("&&")
        first = parts[0].strip()
        if first.startswith("cd "):
            test_dir = first[3:].strip()
        parts = [p.strip() for p in parts if not p.strip().startswith("cd ")]
        cmd = " && ".join(parts)

    cwd = str(ZEPHYR_DIR / test_dir) if test_dir else str(ZEPHYR_DIR)

    env = os.environ.copy()
    env["ZEPHYR_BASE"] = str(ZEPHYR_DIR)
    env["ZEPHYR_SDK_INSTALL_DIR"] = str(SDK_DIR)

    p = subprocess.run(cmd, shell=True, capture_output=True, text=True,
                       errors="replace", timeout=300, cwd=cwd, env=env)

    if log_path:
        log_path = Path(log_path)
        log_path.parent.mkdir(parents=True, exist_ok=True)
        log_path.write_text(
            f"=== COMMAND ===\n{cmd}\n\n"
            f"=== STDOUT ===\n{p.stdout or ''}\n"
            f"=== STDERR ===\n{p.stderr or ''}\n"
            f"=== RC ===\n{p.returncode}\n",
            encoding="utf-8", errors="replace")

    stdout = p.stdout or ""
    stderr = p.stderr or ""
    full_out = stdout + stderr
    tn = task.get("unit_test", "")
    passed = ("PROJECT EXECUTION SUCCESSFUL" in stdout and
              bool(re.search(rf"PASS\s+-\s+{re.escape(tn)}", stdout)) and
              not re.search(rf"FAIL\s+-\s+{re.escape(tn)}", stdout))

    if not passed:
        for line in full_out.split("\n"):
            if "error:" in line.lower() or "FATAL" in line:
                print(f"  {line.strip()[:120]}")
                break

    # 返回完整 stdout+stderr，供 classify 可靠检测编译错误
    # （编译错误常在 stderr，且可能不在输出末尾，截断会漏判）
    return passed, full_out


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--task-id", type=str)
    parser.add_argument("--batch", type=int)
    parser.add_argument("--log", metavar="FILE", default="inject_test.log",
                        help="进度日志文件（默认 inject_test.log）")
    parser.add_argument("--resume", metavar="LOG", nargs="?", const=True,
                        help="断点续跑：跳过日志中已 PASS 的任务；省略 LOG 时用 --log 指定的文件")
    args = parser.parse_args()

    tasks = load_tasks()
    if args.task_id:
        tasks = [t for t in tasks if t["task_id"] == args.task_id]
    if args.batch:
        tasks = tasks[:args.batch]

    # --resume: 从日志恢复，跳过已通过的任务
    skip = set()
    if args.resume:
        resume_file = args.resume if isinstance(args.resume, str) else args.log
        try:
            with open(resume_file, encoding="utf-8", errors="replace") as f:
                for line in f:
                    m = re.match(r"\[\d+/\d+\] task (\S+): .* PASS", line)
                    if m:
                        skip.add(m.group(1))
        except OSError:
            print(f"WARNING: cannot read resume log: {resume_file}")
        tasks = [t for t in tasks if t["task_id"] not in skip]
        print(f"Resume: {len(skip)} tasks already PASS, running {len(tasks)} remaining")

    total = len(tasks)
    passed = 0
    log_path = Path(args.log)
    for i, task in enumerate(tasks):
        git_reset()
        tid, fn = task["task_id"], task["sut_function"]
        print(f"[{i+1}/{total}] task {tid}: {fn}", end=" ", flush=True)

        if not inject_oracle(task):
            print("SKIP (not found)")
            result = "SKIP (not found)"
        else:
            ok, _ = run_test(task)
            result = "PASS" if ok else "FAIL"
            print(result)
            if ok:
                passed += 1

        # 实时追加进度日志，支持 --resume 断点续跑
        with open(log_path, "a", encoding="utf-8") as f:
            f.write(f"[{i+1}/{total}] task {tid}: {fn} {result}\n")

    git_reset()
    print(f"\n{passed}/{total} PASS")


if __name__ == "__main__":
    main()
