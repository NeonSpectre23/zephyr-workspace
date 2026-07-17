#!/usr/bin/env python3
"""注入 oracle → 编译 → 测试，用 git 管理源码状态。

用法:
  python3 inject_test.py --task-id 1
  python3 inject_test.py --batch 5
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
SDK_DIR = Path("/home/huyj/zephyr-sdk-1.0.1")


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


def run_test(task):
    """运行 west build + 测试。"""
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
                       timeout=300, cwd=cwd, env=env)

    stdout = p.stdout or ""
    tn = task.get("unit_test", "")
    passed = ("PROJECT EXECUTION SUCCESSFUL" in stdout and
              bool(re.search(rf"PASS\s+-\s+{re.escape(tn)}", stdout)) and
              not re.search(rf"FAIL\s+-\s+{re.escape(tn)}", stdout))

    if not passed:
        for line in (stdout + (p.stderr or "")).split("\n"):
            if "error:" in line.lower() or "FATAL" in line:
                print(f"  {line.strip()[:120]}")
                break

    return passed, stdout[-300:]


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--task-id", type=str)
    parser.add_argument("--batch", type=int)
    args = parser.parse_args()

    tasks = load_tasks()
    if args.task_id:
        tasks = [t for t in tasks if t["task_id"] == args.task_id]
    if args.batch:
        tasks = tasks[:args.batch]

    total = len(tasks)
    passed = 0
    for i, task in enumerate(tasks):
        git_reset()
        tid, fn = task["task_id"], task["sut_function"]
        print(f"[{i+1}/{total}] task {tid}: {fn}", end=" ", flush=True)

        if not inject_oracle(task):
            print("SKIP (not found)")
            continue

        ok, _ = run_test(task)
        print("PASS" if ok else "FAIL")
        if ok:
            passed += 1

    git_reset()
    print(f"\n{passed}/{total} PASS")


if __name__ == "__main__":
    main()
