#!/usr/bin/env python3
"""Zephyr whitebox 复用流水线(确定性部分)。

对一次模型 run(results.jsonl + trajectory/)自动完成:
  classify   剔除 illegal / harness 假失败(FF) / "实过却判败"假失败,并给每条真实失败修正 L1
  casebook   生成 oracle-vs-Claude 对照(供逐条人工/AI 判 L2/L3)
  prelabel   确定性预标(只有把握的模式 + confidence;低置信不标)
  finalize   annotations(L1/L2/L3) -> 最终 whitebox_report.json(canonical L2 + mechanism + module)
  selftest   classify 逻辑回归自检

口径与 whitebox.py 一致:
  - illegal(illegal_changes 非空)、FF(timeout/watchdog/api_error/exception)不进白盒;
  - 日志含 PROJECT EXECUTION SUCCESSFUL → 实过,剔除(假失败);
  - 细粒度 L2/L3 由人工/AI 填(annotations.jsonl),确定性部分只标有把握的;
  - 报告 classification.L2 = canonical(L2_CONSOLIDATED 合并)。

用法:
  python analyze_run.py classify  --results results-<m>/results.jsonl --trajectory trajectory-<m>
  python analyze_run.py casebook  --results ... --trajectory ... --out casebook.jsonl
  python analyze_run.py prelabel  --casebook casebook.jsonl --out prelabel.jsonl
  python analyze_run.py finalize  --annotations annotations.jsonl --casebook casebook.jsonl --out whitebox_report.json
  python analyze_run.py selftest
"""
import argparse
import json
import re
import sys
from collections import Counter
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

from whitebox import classify_module, L2_CONSOLIDATED, L2_TO_MECH
from annotate_worklist import (  # noqa: E402
    _load_dataset_oracle_map, _oracle_body, _added_lines,
    _diff_section, _err_snippet, _has_ztest_run,
)

FF_CATS = {"timeout", "watchdog", "api_error", "exception"}

# ---------------- ① classify ----------------

def _harness_text(traj_dir: Path, tid: str) -> str:
    """取 Harness Verify 段(判据所在);没有该段(如 api_error)则取全文尾。"""
    p = traj_dir / f"task_{tid}.log"
    if not p.exists():
        return ""
    t = p.read_text(encoding="utf-8", errors="replace")
    if "=== Harness Verify ===" in t:
        return t.split("=== Harness Verify ===", 1)[1]
    return t


def _case_text(traj_dir: Path, tid: str) -> str:
    """裁掉 Claude 会话前缀,保留 Diff + Harness Verify 两段(casebook 判 L2/L3 用)。"""
    p = traj_dir / f"task_{tid}.log"
    if not p.exists():
        return ""
    t = p.read_text(encoding="utf-8", errors="replace")
    first = len(t)
    for m in ("=== Diff ===", "=== Harness Verify ==="):
        i = t.find(m)
        if i != -1 and i < first:
            first = i
    return t[first:]


def classify_output(text: str) -> str:
    """把一段(编译+运行)输出判定成 PASS/compile_error/crash/test_failure/test_not_executed。

    修复后的规则(避免把良性文本判成失败):
      1) PROJECT EXECUTION SUCCESSFUL → PASS(native_sim 全过才打印;未捕获崩溃进程已停);
      2) 编译/链接错误且无任何测试运行标记 → compile_error;
      3) 未捕获崩溃:ZEPHYR FATAL/ASSERTION FAIL 次数 > 'Caught system error'(受控 panic 1:1),
         或出现 Segmentation fault → crash;
      4) 测试跑了且 PROJECT FAILED 或任意 FAIL - → test_failure(不依赖具体 unit_test 名);
      5) 其它 → test_not_executed。
    """
    s = text or ""
    if "PROJECT EXECUTION SUCCESSFUL" in s:
        return "PASS"
    ran = bool(re.search(r"(?:Running TESTSUITE|START - test|PASS - |FAIL - |SUITE (?:PASS|FAIL))", s))
    if re.search(r"(?:BUILD FAILED|ninja: error|undefined reference|"
                 r"\bfatal error:|\berror:)", s) and not ran:
        return "compile_error"
    fatal_events = len(re.findall(r"ZEPHYR FATAL ERROR", s))
    caught = s.count("Caught system error")
    uncaught = (fatal_events > caught) or ("Segmentation fault" in s) or (
        "ASSERTION FAIL" in s and "Caught system error" not in s)
    if uncaught:
        return "crash"
    if ran and ("PROJECT EXECUTION FAILED" in s or re.search(r"FAIL\s+-", s)):
        return "test_failure"
    return "test_not_executed"


def classify_run(results_path, traj_dir):
    """对每个 failed 结果给出修正 L1 与剔除标记,返回真实失败清单。"""
    results = [json.loads(line) for line in open(results_path, encoding="utf-8") if line.strip()]
    traj = Path(traj_dir)
    real, dropped = [], []
    for r in results:
        if r.get("passed"):
            continue
        tid = str(r["task_id"])
        cat = r.get("error_category", "")
        ille = bool(r.get("illegal_changes"))
        ff = cat in FF_CATS
        text = _harness_text(traj, tid)
        log_cat = classify_output(text)
        real_pass = (log_cat == "PASS")
        rec = {
            "task_id": tid,
            "orig_category": cat,
            "log_category": log_cat,
            "illegal": ille, "harness_ff": ff, "real_pass": real_pass,
        }
        if ille or ff or real_pass:
            rec["drop_reason"] = ("illegal" if ille else "harness_ff" if ff else "real_pass")
            dropped.append(rec)
        else:
            # 修正 L1:test_not_executed 多为误标,但保留以便人工核(正常应 0)
            rec["L1"] = log_cat
            real.append(rec)
    return real, dropped


# ---------------- normalize / audit ----------------

def normalize(results_path, traj_dir, backup=True):
    """用修复后的判定重写 results 的 passed/error_category(不重跑, 零算力)。

    优先级: illegal_changes 非空 → 保持 illegal_modifications(指令遵循, 归 results);
    原始类别属 harness FF(timeout/watchdog/api_error/exception) → 保持;
    否则按日志判定: SUCCESS → passed=True/none; 其余 → compile_error/crash/test_failure。
    原始文件备份为 <results>.raw。
    """
    path = Path(results_path)
    rows = [json.loads(line) for line in open(path, encoding="utf-8") if line.strip()]
    if backup:
        raw = path.with_suffix(path.suffix + ".raw")
        if not raw.exists():
            raw.write_bytes(path.read_bytes())
    traj = Path(traj_dir)
    changed = passed_fixed = 0
    for r in rows:
        tid = str(r["task_id"])
        orig = r.get("error_category")
        orig_pass = r.get("passed")
        log_cat = classify_output(_harness_text(traj, tid))
        if r.get("illegal_changes"):
            new_cat, new_pass = "illegal_modifications", orig_pass
        elif orig in FF_CATS:
            new_cat, new_pass = orig, orig_pass
        elif log_cat == "PASS":
            new_cat, new_pass = "none", True
        else:
            new_cat, new_pass = log_cat, False
        if new_cat != orig or new_pass != orig_pass:
            changed += 1
        if new_pass and not orig_pass:
            passed_fixed += 1
        r["error_category"], r["passed"] = new_cat, new_pass
        if new_pass:
            r["error"] = None
    with open(path, "w", encoding="utf-8") as f:
        for r in rows:
            f.write(json.dumps(r, ensure_ascii=False) + "\n")
    n_pass = sum(1 for r in rows if r.get("passed"))
    print(f"normalize {path}: {len(rows)} 条, 改动 {changed} 条, 其中误判失败→通过 {passed_fixed} 条;"
          f" 现 passed {n_pass}/{len(rows)}")
    return rows


def audit(results_path, traj_dir, whitebox_path, out_path=None):
    """口径自查: illegal∩whitebox=∅ / FF 剔除 / 机制映射 / L1 集合。"""
    res = [json.loads(line) for line in open(results_path, encoding="utf-8") if line.strip()]
    wb = json.load(open(whitebox_path, encoding="utf-8"))
    illegal = {str(r["task_id"]) for r in res if r.get("illegal_changes")}
    wb_ids = {w["task_id"] for w in wb}
    real, dropped = classify_run(results_path, traj_dir)
    real_ids = {r["task_id"] for r in real}
    checks = {
        "illegal_and_whitebox_disjoint": len(illegal & wb_ids) == 0,
        "whitebox_equals_classified_real": wb_ids == real_ids,
        "false_failures_removed_from_whitebox": all(
            not w.get("is_false_failure") for w in wb),
        "mechanism_matches_L2_TO_MECH": all(
            w["mechanism"] == L2_TO_MECH.get(w["classification"]["L2"], "misunderstanding")
            for w in wb),
        "L1_set_expected": {w["classification"]["L1"] for w in wb}
        <= {"compile_error", "crash", "test_failure"},
        "no_placeholder_L2": not any(
            w["classification"]["L2"] in ("needs_manual_inspection", "unclassified")
            for w in wb),
    }
    report = {
        "results": str(results_path),
        "whitebox": str(whitebox_path),
        "n_results": len(res), "n_passed": sum(1 for r in res if r.get("passed")),
        "n_illegal(results)": len(illegal),
        "n_harness_ff": sum(1 for d in dropped if d["drop_reason"] == "harness_ff"),
        "n_real_pass_excluded": sum(1 for d in dropped if d["drop_reason"] == "real_pass"),
        "n_whitebox": len(wb),
        "L1": dict(Counter(w["classification"]["L1"] for w in wb)),
        "mechanism": dict(Counter(w["mechanism"] for w in wb)),
        "checks": checks,
        "all_pass": all(checks.values()),
    }
    if out_path:
        Path(out_path).write_text(json.dumps(report, ensure_ascii=False, indent=2), encoding="utf-8")
    print(json.dumps(report, ensure_ascii=False, indent=2))
    return report


# ---------------- ② casebook ----------------

def gen_casebook(results_path, traj_dir, out_path):
    real, _ = classify_run(results_path, traj_dir)
    results = {str(json.loads(line)["task_id"]): json.loads(line)
               for line in open(results_path, encoding="utf-8") if line.strip()}
    om = _load_dataset_oracle_map()
    traj = Path(traj_dir)
    rows = []
    for r in real:
        tid = r["task_id"]
        res = results[tid]
        task = res.get("snapshot", {}).get("task", {}) or {}
        sut, src = task.get("sut_function", ""), task.get("source_path", "")
        text = _case_text(traj, tid)
        oracle_rel = om.get((sut, src), "") or ""
        rows.append({
            "task_id": tid,
            "error_category": r["L1"],
            "sut_function": sut,
            "source_path": src,
            "module": classify_module(src),
            "oracle": oracle_rel,
            "oracle_body": _oracle_body(oracle_rel, sut),
            "claude_added": _added_lines(_diff_section(text)),
            "err_snip": _err_snippet(r["L1"], text),
            "has_ztest_run": _has_ztest_run(text),
        })
    Path(out_path).parent.mkdir(parents=True, exist_ok=True)
    with open(out_path, "w", encoding="utf-8") as f:
        for x in rows:
            f.write(json.dumps(x, ensure_ascii=False) + "\n")
    print(f"casebook → {out_path}  ({len(rows)} 真实失败)")
    return rows


# ---------------- ③ prelabel(确定性、低噪声) ----------------

_NULL_GUARD = re.compile(r"(?:if|while|CHECKIF|__ASSERT).{0,40}(?:NULL|==\s*0|!=\s*NULL)")


def prelabel_case(c: dict) -> list:
    """只标有把握的模式,附 confidence;判不出返回 []。"""
    cat = c.get("error_category", "")
    err = (c.get("err_snip") or "").lower()
    out = []
    if cat == "compile_error":
        if "undefined reference" in err:
            out.append({"L2": "undefined_reference", "confidence": "high",
                        "basis": "undefined reference"})
        elif "implicit declaration" in err or "undeclared" in err:
            out.append({"L2": "hallucinated_api", "confidence": "high",
                        "basis": "implicit declaration/undeclared"})
    elif cat == "crash":
        oracle = c.get("oracle_body") or ""
        claude = "\n".join(c.get("claude_added") or [])
        o_null = len(_NULL_GUARD.findall(oracle))
        c_null = len(_NULL_GUARD.findall(claude))
        if o_null > c_null:
            out.append({"L2": "missing_null_check", "confidence": "medium",
                        "basis": f"oracle 空指针保护 {o_null} 处 > claude {c_null} 处"})
    return out


# ---------------- ⑥ finalize ----------------

def finalize(annotations_path, casebook_path, out_path):
    ann = {json.loads(line)["task_id"]: json.loads(line)
           for line in open(annotations_path, encoding="utf-8") if line.strip()}
    cb = {json.loads(line)["task_id"]: json.loads(line)
          for line in open(casebook_path, encoding="utf-8") if line.strip()}
    rows = []
    for tid in sorted(ann, key=int):
        a, c = ann[tid], cb[tid]
        l2 = L2_CONSOLIDATED.get(a["L2"], a["L2"])
        rows.append({
            "task_id": tid,
            "sut_function": c.get("sut_function", ""),
            "source_path": c.get("source_path", ""),
            "module": classify_module(c.get("source_path", "")),
            "classification": {"L1": a.get("L1", c.get("error_category", "")),
                               "L2": l2, "L3_detail": a.get("L3", [])},
            "mechanism": L2_TO_MECH.get(l2, "misunderstanding"),
            "is_false_failure": False,
            "actual_pass": False,
        })
    Path(out_path).parent.mkdir(parents=True, exist_ok=True)
    with open(out_path, "w", encoding="utf-8") as f:
        json.dump(rows, f, ensure_ascii=False, indent=2)
    print(f"whitebox_report → {out_path}  ({len(rows)} 条)")
    print("L1:", dict(Counter(w["classification"]["L1"] for w in rows)))
    print("L2:", dict(Counter(w["classification"]["L2"] for w in rows)))
    print("mechanism:", dict(Counter(w["mechanism"] for w in rows)))
    return rows


# ---------------- CLI ----------------

def _selftest():
    cases = [
        ("受控 panic 全过", "ASSERTION FAIL\nZEPHYR FATAL ERROR 4\nCaught system error\nPASS - p\nPROJECT EXECUTION SUCCESSFUL", "PASS"),
        ("良性 panic 测试名", "START - test_log_panic\nPASS - test_log_panic\nPROJECT EXECUTION SUCCESSFUL", "PASS"),
        ("SUCCESS 但含 benign segfault 文本", "Segmentation fault occurs only if ...\nPROJECT EXECUTION SUCCESSFUL", "PASS"),
        ("未捕获 panic", "ASSERTION FAIL\nZEPHYR FATAL ERROR 4\nExiting due to fatal error", "crash"),
        ("段错误", "Running TESTSUITE a\nSTART - t\nSegmentation fault (core dumped)", "crash"),
        ("真实 FAIL 不匹配目标名", "FAIL - test_encode\nSUITE FAIL\nPROJECT EXECUTION FAILED", "test_failure"),
        ("编译错无测试", "ninja: error\nundefined reference to `x'", "compile_error"),
        ("目标 PASS 但另有 FAIL", "PASS - test_a\nFAIL - test_b\nPROJECT EXECUTION FAILED", "test_failure"),
        ("空输出", "Loading Zephyr default modules", "test_not_executed"),
    ]
    bad = 0
    for name, text, exp in cases:
        got = classify_output(text)
        if got != exp:
            bad += 1
            print(f"  [FAIL] {name}: exp {exp} got {got}")
    print("selftest:", "ALL OK" if bad == 0 else f"{bad} FAILED")
    sys.exit(1 if bad else 0)


def main():
    ap = argparse.ArgumentParser(description="Zephyr whitebox 复用流水线")
    sub = ap.add_subparsers(dest="cmd", required=True)

    p = sub.add_parser("classify", help="剔 illegal/FF/实过,给真实失败修正 L1")
    p.add_argument("--results", required=True)
    p.add_argument("--trajectory", required=True)

    p = sub.add_parser("casebook", help="生成 oracle-vs-Claude 对照")
    p.add_argument("--results", required=True)
    p.add_argument("--trajectory", required=True)
    p.add_argument("--out", required=True)

    p = sub.add_parser("prelabel", help="确定性预标")
    p.add_argument("--casebook", required=True)
    p.add_argument("--out", required=True)

    p = sub.add_parser("finalize", help="annotations -> whitebox_report")
    p.add_argument("--annotations", required=True)
    p.add_argument("--casebook", required=True)
    p.add_argument("--out", required=True)

    p = sub.add_parser("normalize", help="按修复后判定重写 results 的 passed/error_category")
    p.add_argument("--results", required=True)
    p.add_argument("--trajectory", required=True)

    p = sub.add_parser("audit", help="口径自查(illegal∩whitebox/FF/机制映射/L1)")
    p.add_argument("--results", required=True)
    p.add_argument("--trajectory", required=True)
    p.add_argument("--whitebox", required=True)
    p.add_argument("--out", default="")

    sub.add_parser("selftest", help="分类回归自检")
    a = ap.parse_args()

    if a.cmd == "selftest":
        _selftest()
    elif a.cmd == "normalize":
        normalize(a.results, a.trajectory)
    elif a.cmd == "audit":
        audit(a.results, a.trajectory, a.whitebox, a.out or None)
    elif a.cmd == "classify":
        real, dropped = classify_run(a.results, a.trajectory)
        c1 = Counter(r["L1"] for r in real)
        cd = Counter(d["drop_reason"] for d in dropped)
        print(f"真实失败 {len(real)} 条: {dict(c1)}")
        print(f"剔除 {len(dropped)} 条: {dict(cd)}")
    elif a.cmd == "casebook":
        gen_casebook(a.results, a.trajectory, a.out)
    elif a.cmd == "prelabel":
        hits = 0
        with open(a.out, "w", encoding="utf-8") as f:
            for line in open(a.casebook, encoding="utf-8"):
                if not line.strip():
                    continue
                c = json.loads(line)
                cands = prelabel_case(c)
                rec = {"task_id": c["task_id"], "L1": c.get("error_category", ""),
                       "candidates": cands,
                       "needs_llm": not any(x["confidence"] == "high" for x in cands)}
                if cands:
                    hits += 1
                f.write(json.dumps(rec, ensure_ascii=False) + "\n")
        print(f"prelabel → {a.out}: {hits} 条有候选(其余 needs_llm)")
    elif a.cmd == "finalize":
        finalize(a.annotations, a.casebook, a.out)


if __name__ == "__main__":
    main()
