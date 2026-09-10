#!/usr/bin/env python3
"""生成 96 条失败样本的"病历"casebook,供逐条人工/AI 标注 L2 + L3 用。

每个 case 拼出:
  - 任务标识 + error_category + module
  - oracle 函数体(来自 zephyr-bench/oracles/<id>.{c,h})
  - Claude 改动行(从轨迹 '=== Diff ===' 的 + 行还原——mask 只替换函数体,
    故 + 行 ≈ Claude 补全的函数体)
  - err_snip:按类别抽取的报错/测试输出片段(compile 取编译错、crash 取崩溃、
    test_failure 取 FAIL 断言、test_not_executed 取执行尾部)

输出: results-deepseek-v4-pro/annotation/casebook.jsonl(每行一个 case)
用法:
  python annotate_worklist.py [--results results-deepseek-v4-pro/results.jsonl]
"""
import argparse
import json
import re
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
from whitebox import classify_module, _false_failure_reason

BASE = Path(__file__).resolve().parent
BENCH = BASE.parent / "zephyr-bench"
DEFAULT_RESULTS = BASE / "results-deepseek-v4-pro" / "results.jsonl"
DEFAULT_TRAJ = BASE / "trajectory-deepseek-v4-pro"
OUT_DIR = BASE / "results-deepseek-v4-pro" / "annotation"

CRASH_MARK = re.compile(
    r"Segmentation fault|segfault|Aborted|panic|k_panic|assert|FAIL\s*-", re.I)
ERR_LINE = re.compile(
    r"error:|undefined reference|implicit declaration|undeclared|-Werror|warning:")


def _diff_section(traj_text: str) -> str:
    if "=== Diff ===" in traj_text:
        return traj_text.split("=== Diff ===", 1)[1]
    return ""


def _added_lines(diff: str, cap: int = 150):
    """unified diff 里的 + 行(不含 +++ 头) ≈ Claude 补全内容。"""
    out = []
    for ln in diff.splitlines():
        if ln.startswith("+") and not ln.startswith("+++"):
            out.append(ln[1:])
        if len(out) >= cap:
            out.append("... (truncated)")
            break
    return out


def _err_snippet(cat: str, traj_text: str) -> str:
    lines = traj_text.splitlines()
    picks = []
    if cat == "compile_error":
        for i, ln in enumerate(lines):
            if ERR_LINE.search(ln):
                picks.append(ln.strip()[:300])
        return "\n".join(picks[:12])
    if cat == "crash":
        for i, ln in enumerate(lines):
            if CRASH_MARK.search(ln):
                lo = max(0, i - 2)
                picks.append("\n".join(x.strip()[:300] for x in lines[lo:i + 4]))
                break
        return "\n".join(picks[:1])[:2500]
    if cat == "test_failure":
        for ln in lines:
            s = ln.strip()
            if re.search(r"FAIL\s*-|Assertion|expected|actual|EXPECTED", s):
                picks.append(s[:300])
        return "\n".join(picks[:14])
    if cat == "test_not_executed":
        # 复核用:agent 判 test_not_executed 可能是"目标 unit_test 名没匹配上"
        # 而把真实 test_failure 误标。这里把 ztest 运行证据(有没有跑起来)抓出来:
        # 有 RUNNING/START/SUITE/FAIL/PASS → 测试其实执行了 → 应按 test_failure 标。
        run = [x.strip()[:200] for x in lines
               if re.search(r"Running TESTSUITE|START - |PASS - |FAIL - |SUITE |PROJECT EXECUTION", x)]
        tail = [x.strip()[:200] for x in lines[-4:]]
        return "\n".join(run[-20:] + tail)
    return "\n".join(x.strip()[:200] for x in lines[:15])


def _has_ztest_run(traj_text: str) -> bool:
    """测试是否真正执行过(有 ztest PASS/FAIL/START/SUITE 输出)。"""
    return bool(re.search(
        r"(START - test|PASS - |FAIL - |SUITE (PASS|FAIL))", traj_text or ""))


def _oracle_body(oracle_rel: str, sut_function: str) -> str:
    """读 oracle 文件并抽出 sut 函数体(oracle_rel 相对 zephyr-bench/)。"""
    if not oracle_rel:
        return ""
    p = BENCH / oracle_rel
    if not p.exists():
        return ""
    src = p.read_text(encoding="utf-8", errors="replace")
    try:
        from mask import extract_function_body
        b = extract_function_body(src, sut_function)
        return b or ""
    except Exception:
        return src[:2000]


def _load_dataset_oracle_map():
    """当前 712 数据集:(sut_function, source_path) -> oracle 相对路径。

    results.jsonl 的 snapshot.task.oracle 可能是旧编号(重映射前),不可信;
    一律按 (sut, source_path) 反查当前数据集,保证与 712 口径一致。
    """
    m = {}
    for f in ("zephyr_tasks.c.jsonl", "zephyr_tasks.h.jsonl"):
        p = BENCH / f
        if not p.exists():
            continue
        for line in open(p, encoding="utf-8"):
            line = line.strip()
            if not line:
                continue
            t = json.loads(line)
            m[(t["sut_function"], t["source_path"])] = t.get("oracle", "")
    return m


def build():
    ap = argparse.ArgumentParser()
    ap.add_argument("--results", default=str(DEFAULT_RESULTS))
    ap.add_argument("--trajectory", default=str(DEFAULT_TRAJ))
    args = ap.parse_args()

    traj_dir = Path(args.trajectory)
    results = []
    for line in open(args.results, encoding="utf-8"):
        line = line.strip()
        if line:
            results.append(json.loads(line))

    failed = [r for r in results if not r.get("passed")]
    # 与 whitebox 完全一致:illegal(指令遵循, 在 results) 与 FF(_false_failure_reason
    # 判定:timeout/watchdog/api_error/exception/utf8 等)不进 casebook → 应得 96。
    def _excluded(r):
        traj_text = ""
        tp = traj_dir / f"task_{r['task_id']}.log"
        if tp.exists():
            traj_text = tp.read_text(encoding="utf-8", errors="replace")
        return bool(r.get("illegal_changes")) or bool(_false_failure_reason(r, traj_text))

    real = [r for r in failed if not _excluded(r)]
    print(f"results {len(results)} 行, failed {len(failed)}, 真实失败(入 casebook) {len(real)}")

    oracle_map = _load_dataset_oracle_map()
    OUT_DIR.mkdir(parents=True, exist_ok=True)
    out = OUT_DIR / "casebook.jsonl"
    n = 0
    with open(out, "w", encoding="utf-8") as f:
        for r in real:
            tid = str(r["task_id"])
            task = r.get("snapshot", {}).get("task", {}) or {}
            cat = r.get("error_category", "")
            traj_text = ""
            tp = traj_dir / f"task_{tid}.log"
            if tp.exists():
                traj_text = tp.read_text(encoding="utf-8", errors="replace")
                # 只留 Diff + Harness Verify 两段,裁掉巨大的 Claude 会话前缀
                first = len(traj_text)
                for m in ("=== Diff ===", "=== Harness Verify ==="):
                    i = traj_text.find(m)
                    if i != -1 and i < first:
                        first = i
                traj_text = traj_text[first:]
            sut = task.get("sut_function", "")
            src = task.get("source_path", "")
            # oracle 以当前 712 数据集反查为准(不信 snapshot 里的旧编号路径)
            oracle_rel = oracle_map.get((sut, src), "") or ""
            oracle_body = _oracle_body(oracle_rel, sut)
            added = _added_lines(_diff_section(traj_text)) or []
            case = {
                "task_id": tid,
                "error_category": cat,
                "sut_function": sut,
                "source_path": src,
                "module": classify_module(src),
                "oracle": oracle_rel,
                "oracle_body": oracle_body,
                "claude_added": added,
                "err_snip": _err_snippet(cat, traj_text),
                "has_ztest_run": _has_ztest_run(traj_text),
            }
            f.write(json.dumps(case, ensure_ascii=False) + "\n")
            n += 1
    print(f"casebook → {out}  ({n} cases)")


if __name__ == "__main__":
    build()
