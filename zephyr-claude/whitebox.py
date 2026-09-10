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
import sys
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

# ---- 机制维度权威映射(整改清单 §四:完全共用 L2_CONSOLIDATED + L2_TO_MECH) ----
# L2_TO_MECH 与 collab Closed_source/QSemOS/qsem-claude/analysis/failure_crosstab.py
# 的权威映射逐项一致(single source of truth),不要本地自行增删。
#   若 L2 出现在 L2_CONSOLIDATED 的合并后集合里,也用本表映射(表中含全部 canonical 键)。
#   integrity_violation 不应出现在 whitebox——指令遵循失败只在 results.jsonl 用
#   illegal_changes 记录(见 failure_crosstab.py infer_mechanism)。
L2_TO_MECH = {
    # -- hallucination(编造不存在的符号/API/类型 → 编译失败) --
    'undefined_reference': 'hallucination',
    'hallucinated_api': 'hallucination',
    'hallucinated_symbol': 'hallucination',
    'hallucinated_api_and_wrong_architecture': 'hallucination',
    'implicit_declaration': 'hallucination',

    # -- misunderstanding(其余全部) --
    'missing_include': 'misunderstanding',
    'type_error': 'misunderstanding',
    'mixed': 'misunderstanding',
    'werror_style': 'misunderstanding',
    'missing_conditional_guard': 'misunderstanding',
    'extra_conditional_guard': 'misunderstanding',
    'wrong_calculation': 'misunderstanding',
    'missing_synchronization': 'misunderstanding',
    'wrong_error_handling': 'misunderstanding',
    'data_structure_misuse': 'misunderstanding',
    'semantic_drift': 'misunderstanding',
    'missing_null_check': 'misunderstanding',
    'wrong_operator': 'misunderstanding',
    'semantic_misunderstanding': 'misunderstanding',
    'algorithm_incomplete': 'misunderstanding',
    'side_effect_error': 'misunderstanding',
    'missing_corner_cases': 'misunderstanding',
    'control_flow_error': 'misunderstanding',
    'accumulator_precision': 'misunderstanding',
    'bit_manipulation_error': 'misunderstanding',
    'architecture_error': 'misunderstanding',
    'loop_logic_error': 'misunderstanding',
    'data_structure_order': 'misunderstanding',
    'mixed_guard_and_loop': 'misunderstanding',
    'validation_logic_inverted': 'misunderstanding',
    'wrong_return_value': 'misunderstanding',
    'wrong_error_semantics': 'misunderstanding',
    'missing_api_call': 'misunderstanding',
    'test_not_executed': 'misunderstanding',
    'needs_manual_inspection': 'misunderstanding',
    'unclassified': 'misunderstanding',
    'logic_deviation': 'misunderstanding',
    'missing_error_path': 'misunderstanding',
    'empty_stub': 'misunderstanding',
    'unknown_crash': 'misunderstanding',
    'hardware_knowledge_gap': 'misunderstanding',
    'incomplete_implementation': 'misunderstanding',
    'bitmask_error': 'misunderstanding',
    'missing_context_switch': 'misunderstanding',
}

# 自检(整改清单 §四:两表完全共用):L2_CONSOLIDATED 的合并结果必须都能用权威
# L2_TO_MECH 映射到机制;否则说明本文件与 failure_crosstab.py 发生漂移,直接报错。
# 例外:harness_bug_*(假失败)不在权威表内——FF 写盘前已剔除、不参与机制映射。
_CONSOLIDATED_UNMAPPED = sorted(
    v for v in set(L2_CONSOLIDATED.values())
    if not v.startswith("harness_bug") and v not in L2_TO_MECH)
if _CONSOLIDATED_UNMAPPED:
    raise RuntimeError(
        "L2_CONSOLIDATED 合并结果在权威 L2_TO_MECH 中缺映射: "
        f"{_CONSOLIDATED_UNMAPPED} —— 需与 analysis/failure_crosstab.py 对齐")

# ---- Zephyr 模块 L1 划分(整改清单 §四「已落实」:OS 模块维度) ----
# 权威 L1 集合 = {kernel, subsys, lib, drivers, arch, <subsystem>, modules, samples, other}
#   - 顶层目录(除 include)直接作为 L1;
#   - include/zephyr/<sub>/... 映射到 <sub>(头文件内联函数归入其子系统);
#   - include/zephyr/<file>.h(无子系统子目录)与其它无法归类者 → 'other'。
# ⚠️ 与 QSem 的 classify_module(关键字匹配)不同——Zephyr 目录结构不适用,勿照搬。
ZEPHYR_MODULE_L1 = {
    "kernel", "subsys", "lib", "drivers", "arch",
    "modules", "samples", "other",
}

# include/zephyr/ 下直接挂着的顶层头文件(无子系统子目录)→ 归到实际子系统,
# 避免全部掉进 'other'。本表只收录当前数据集实际命中的头文件,新增时按需扩展。
ZEPHYR_TOP_HEADER_MODULE = {
    "net_buf.h": "net",     # net_buf 网络缓冲(实现于 subsys/net/buf)
    "kernel.h": "kernel",
    "device.h": "kernel",   # 驱动模型
    "spinlock.h": "kernel",
}


def classify_module(source_path: str) -> str:
    """给出 Zephyr 模块 L1 划分(整改清单 §四「已落实」)。

    - 顶层目录(除 include)直接作为 L1;
    - include/zephyr/<sub>/<file>... → 映射到 <sub>(头文件内联函数归到其子系统);
    - include/zephyr/<file>.h(已知顶层头文件)→ 按 ZEPHYR_TOP_HEADER_MODULE 归入子系统;
    - 其它无法归类 → 'other'。
    """
    if not source_path:
        return "other"
    parts = source_path.split("/")
    top = parts[0]
    if top == "include":
        # include/zephyr/<sub>/<file>... 取第一个非 zephyr、且非文件名的段为 <sub>
        for p in parts[1:]:
            if p == "zephyr":
                continue
            if "." in p:          # 是文件名(如 kernel.h),不是子系统目录
                return ZEPHYR_TOP_HEADER_MODULE.get(p, "other")
            return p
        return "other"
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


def _guard_break(low: str) -> bool:
    """是否破坏 #if/#endif 条件编译配对(理解偏差)。

    只在整个输出出现 unterminated/preprocessor,或某一行**同时**含
    预处理指示符与 error/without/expected/missing 等报错关键词时才判真;
    避免日志里无辜出现的 `#endif` / `#else` 误触发。
    """
    if "unterminated" in low or "preprocessor" in low:
        return True
    for line in low.splitlines():
        if re.search(r"#\s*(?:else|endif|if)\b", line) and re.search(
                r"\b(error|without|expected|missing)\b", line):
            return True
    return False


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
        if _guard_break(low):
            return "missing_conditional_guard"  # 理解偏差:破坏条件编译
        if "werror" in low or "-werror" in low:
            return "werror_style"               # 警告被当作错误
        return "needs_manual_inspection"

    if cat == "crash":
        # crash 根因(漏空指针/缺同步/越界…)无法仅从报错文本断定;
        # 与 RIOT 参考一致(crash_map 全留 needs_manual_inspection),
        # 精确 L2 交由逐例读 diff 标注。
        return "needs_manual_inspection"

    if cat == "test_failure":
        # 编译过但测试失败——无法仅从报错文本判断机理,归入需人工
        return "needs_manual_inspection"

    if cat == "test_not_executed":
        return "test_not_executed"

    return "needs_manual_inspection"


def _false_failure_reason(result: dict, traj_text: str) -> str:
    """harness 级误判的原因标签;非假失败返回 ''。

    假失败剔除、不计入失败(整改清单 §四 铁律①:L2 = harness_bug_*)。
    返回的 reason 用于生成粒度标签,如 harness_bug_api_error / harness_bug_utf8_decode。
    """
    cat = result.get("error_category", "")
    if cat in ("timeout", "watchdog", "api_error", "exception"):
        return cat
    low = (traj_text or "").lower()
    if "utf-8" in low and ("decode" in low or "invalid" in low or "codec" in low):
        return "utf8_decode"
    return ""


def build_report(results_path, traj_dir, out_path):
    results = []
    malformed = 0
    with open(results_path, encoding="utf-8") as f:
        for line in f:
            line = line.strip()
            if not line:
                continue
            try:
                results.append(json.loads(line))
            except json.JSONDecodeError:
                malformed += 1
    if malformed:
        print(f"WARNING: 跳过 {malformed} 行无法解析的 results 行")

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
        ff_reason = _false_failure_reason(r, traj)
        ff = bool(ff_reason)
        l1 = "false_failure_detection" if ff else (r.get("error_category") or "unknown")
        # 假失败给真实粒度标签(harness_bug_<reason>),而非字面 harness_bug_*
        raw_l2 = f"harness_bug_{ff_reason}" if ff else infer_L2(r, traj)
        l2 = L2_CONSOLIDATED.get(raw_l2, raw_l2)
        # classification.L3_detail: 合并前的细粒度根因(如 missing_null_check)
        l3_detail = [raw_l2] if raw_l2 else []

        # mechanism 派生:共用权威 taxonomy(整改清单 §四,与 failure_crosstab.py
        # infer_mechanism 一致):is_false_failure -> 'false_failure',
        # 其余按 L2_TO_MECH 映射(缺省 misunderstanding)。
        mechanism = "false_failure" if ff else L2_TO_MECH.get(l2, "misunderstanding")

        # module + 顶层标识:Zephyr 模块 L1 划分(整改清单 §四)
        snap_task = r.get("snapshot", {}).get("task", {}) or {}
        src = snap_task.get("source_path", "")
        fn = snap_task.get("sut_function", "")
        module = classify_module(src)

        wb.append({
            "task_id": tid,
            "sut_function": fn,
            "source_path": src,
            "module": module,
            "classification": {"L1": l1, "L2": l2, "L3_detail": l3_detail},
            "mechanism": mechanism,
            "is_false_failure": ff,
            "actual_pass": False,
        })

    # 铁律①(§四):is_false_failure(harness_bug_*) 整条剔除、不计入失败。
    # 与 qsem/riot 的 canonical whitebox 文件一致:假失败不落盘,只从 stdout 报告剔除数。
    real = [w for w in wb if not w["is_false_failure"]]
    ff = [w for w in wb if w["is_false_failure"]]

    # 写输出(只写真实失败)
    out_path = Path(out_path)
    out_path.parent.mkdir(parents=True, exist_ok=True)
    with open(out_path, "w", encoding="utf-8") as f:
        json.dump(real, f, ensure_ascii=False, indent=2)

    # 统计(真实失败,不含 FF/illegal)
    l1c = Counter(w["classification"]["L1"] for w in real)
    l2c = Counter(w["classification"]["L2"] for w in real)
    mechc = Counter(w["mechanism"] for w in real)
    modc = Counter(w["module"] for w in real)

    print(f"=== whitebox report ({len(real)} real failures; 剔除 {len(ff)} 假失败) ===")
    print(f"  指令遵循失败(illegal_changes, 在 results.jsonl,不进 whitebox): {n_illegal}")
    print(f"  mechanism: {dict(mechc)}")
    print(f"  L1: {dict(l1c)}")
    print(f"  L2 ({len(l2c)} types):")
    for k, v in l2c.most_common():
        print(f"    {k}: {v}")
    print(f"  module: {dict(modc)}")
    print(f"\nResults saved to {out_path}")


def main():
    # Windows/GBK 控制台中文输出不乱码
    try:
        sys.stdout.reconfigure(encoding="utf-8")
    except Exception:
        pass
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
