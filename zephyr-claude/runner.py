"""
Zephyr-Claude CLI 入口

适配自 riot-claude/runner.py。
"""
import json
import argparse
from pathlib import Path

from config import *
from agent import ClaudeAgent


RESULT_FILE = RESULTS_DIR / "results.jsonl"
TASK_FILES = [Path(f) for f in DEFAULT_TASK_FILES]


def load_task_by_id(task_id, task_files=None):
    if task_files is None:
        task_files = TASK_FILES
    for path in task_files:
        with open(path, "r", encoding="utf-8") as f:
            for line in f:
                if not line.strip():
                    continue
                try:
                    obj = json.loads(line)
                except json.JSONDecodeError:
                    continue
                if str(obj["task_id"]) == str(task_id):
                    return obj
    return None


def load_finished_tasks():
    if not RESULT_FILE.exists():
        return set()
    finished = set()
    with open(RESULT_FILE, "r", encoding="utf-8") as f:
        for line in f:
            if not line.strip():
                continue
            try:
                obj = json.loads(line)
            except json.JSONDecodeError:
                continue
            if obj.get("passed"):
                finished.add(str(obj["task_id"]))
    return finished


def append_result(result):
    RESULT_FILE.parent.mkdir(exist_ok=True)
    with open(RESULT_FILE, "a", encoding="utf-8") as f:
        f.write(json.dumps(result, ensure_ascii=False))
        f.write("\n")


def run_task(agent, task, dataset_paths=None):
    print(f"\n[TASK {task['task_id']}] {task['sut_function']}")
    try:
        result = agent.run(task, dataset_paths=dataset_paths)
    except Exception as e:
        result = {
            "task_id": task["task_id"],
            "passed": False, "runtime_s": -1,
            "illegal_changes": [],
            "error": str(e), "error_category": "exception",
        }
        print(f"[ERROR] {e}")
    append_result(result)
    print("PASS" if result["passed"] else "FAIL")
    return result


def _batch_main(task_files, agent, n, resume=False, start_task_id=None):
    finished = load_finished_tasks() if resume else set()
    tasks = []
    collecting = start_task_id is None
    start_num = int(start_task_id) if start_task_id else 0
    for path in task_files:
        with open(path, "r", encoding="utf-8") as f:
            for line in f:
                if not line.strip():
                    continue
                try:
                    obj = json.loads(line)
                except json.JSONDecodeError:
                    continue
                tid = str(obj["task_id"])
                if resume and tid in finished:
                    print(f"  [SKIP] task {tid} already finished")
                    continue
                if not collecting:
                    if int(tid) >= start_num:
                        collecting = True
                    else:
                        continue
                tasks.append(obj)
                if len(tasks) >= n:
                    break
        if len(tasks) >= n:
            break

    print(f"Batch: {len(tasks)} tasks{', resume mode' if resume else ''}")
    print()

    results = []
    for task in tasks:
        results.append(run_task(agent, task, dataset_paths=task_files))

    print(f"\n{'='*50}")
    print(f"SUMMARY ({len(results)} tasks)")
    print(f"{'='*50}")
    passed = sum(1 for r in results if r["passed"])
    failed = sum(1 for r in results if not r["passed"])
    print(f"PASS: {passed}, FAIL: {failed}")
    for r in results:
        status = "PASS" if r["passed"] else "FAIL"
        err = f" [{r.get('error', '')}]" if r.get("error") else ""
        print(f"  task {r['task_id']}: {status}{err} ({r['runtime_s']}s)")


def run_checks():
    import subprocess
    import sys
    all_ok = True

    def check(label, ok, detail=""):
        nonlocal all_ok
        status = "OK" if ok else "FAIL"
        msg = f"  [{status}] {label}"
        if detail and not ok:
            msg += f"  — {detail}"
        print(msg)
        if not ok:
            all_ok = False

    print("=== Zephyr-Claude Dependency Check ===\n")

    # 1. Docker
    r = subprocess.run(["docker", "info"], capture_output=True, text=True)
    check("Docker daemon", r.returncode == 0, "Is Docker Desktop started?")

    # 2. Docker image
    r = subprocess.run(
        ["docker", "image", "inspect", DOCKER_IMAGE],
        capture_output=True, text=True,
    )
    check(f"Docker image '{DOCKER_IMAGE}'", r.returncode == 0,
          f"Run: docker build -t {DOCKER_IMAGE} .")

    # 3. tree-sitter (new API, uses pip package)
    try:
        import tree_sitter_c
        from tree_sitter import Language, Parser
        _C_LANG = Language(tree_sitter_c.language())
        _PARSER = Parser(_C_LANG)
        _PARSER.parse(b'int foo(void) { return 1; }')
        check("tree-sitter (pip package)", True)
    except Exception as e:
        check("tree-sitter (pip package)", False, str(e))

    # 4. Claude settings
    check("Claude settings", CLAUDE_SETTINGS_SRC.exists())

    # 5. Zephyr repo
    expected = ["CMakeLists.txt", "west.yml", "VERSION", "arch", "kernel"]
    missing = [d for d in expected if not (ZEPHYR_BASE / d).exists()]
    check("Zephyr repo structure", len(missing) == 0,
          f"Missing: {missing}")

    # 6. Zephyr SDK
    check("Zephyr SDK", ZEPHYR_SDK_DIR.exists(),
          f"Expected at {ZEPHYR_SDK_DIR}")

    # 7. Dataset files
    for f in DEFAULT_TASK_FILES:
        exists = Path(f).exists()
        check(f"Dataset '{Path(f).name}'", exists)

    print()
    if all_ok:
        print("All checks passed.")
    else:
        print("Some checks failed.")
        sys.exit(1)


def main():
    parser = argparse.ArgumentParser(description="Zephyr-Claude Benchmark Runner")
    parser.add_argument("--task-id", type=str, help="Run single task")
    parser.add_argument("--batch", type=int, help="Run first N tasks")
    parser.add_argument("--start", type=str, default=None,
                        help="Start from task_id (with --batch)")
    parser.add_argument("--resume", action="store_true",
                        help="Skip already-passed tasks")
    parser.add_argument("--data", type=str, nargs="+",
                        help="Custom dataset files")
    parser.add_argument("--check", action="store_true",
                        help="Verify all dependencies and exit")
    args = parser.parse_args()

    if args.check:
        run_checks()
        return

    if not args.task_id and not args.batch:
        parser.error("Either --task-id or --batch is required")

    task_files = [Path(f) for f in args.data] if args.data else TASK_FILES
    agent = ClaudeAgent(WORKSPACE_ROOT)

    if args.batch:
        _batch_main(task_files, agent, args.batch,
                    resume=args.resume, start_task_id=args.start)
        return

    if args.resume:
        finished = load_finished_tasks()
        if str(args.task_id) in finished:
            print(f"[SKIP] task {args.task_id} already finished")
            return

    task = load_task_by_id(args.task_id, task_files)
    if task is None:
        print(f"[ERROR] task not found: {args.task_id}")
        return

    run_task(agent, task, dataset_paths=task_files)


if __name__ == "__main__":
    main()
