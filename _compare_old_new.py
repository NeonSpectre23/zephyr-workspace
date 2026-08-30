#!/usr/bin/env python3
"""对比新旧数据集:按 sut_function 匹配,比较除 task_id/oracle 编号外的字段 + oracle 内容。"""
import json
import difflib
from pathlib import Path
from collections import Counter, defaultdict

BENCH = Path(__file__).resolve().parent / "zephyr-bench"

COMPARE_FIELDS = ["source_path", "masked_code", "unit_test", "run_command",
                  "suite", "zephyr_test_module"]

def load_tasks(paths):
    d = defaultdict(list)
    for p in paths:
        for line in p.open(encoding="utf-8"):
            line = line.strip()
            if not line:
                continue
            t = json.loads(line)
            d[t["sut_function"]].append(t)
    return d

def oracle_text(task, folder):
    rel = task.get("oracle", "")
    name = Path(rel).name
    p = BENCH / folder / name
    if p.exists():
        return p.read_text(encoding="utf-8", errors="replace")
    return None

def norm(s):
    """规范化:去首尾空白、去行首尾空白,用于忽略纯空白/换行差异。"""
    if s is None:
        return None
    return "\n".join(l.rstrip() for l in s.splitlines()).strip()

old = load_tasks([BENCH / "zephyr_tasks.c-normal.jsonl", BENCH / "zephyr_tasks.h-normal.jsonl"])
new = load_tasks([BENCH / "zephyr_tasks.c.jsonl", BENCH / "zephyr_tasks.h.jsonl"])

old_names = set(old)
new_names = set(new)
common = old_names & new_names
only_old = sorted(old_names - new_names)
only_new = sorted(new_names - old_names)

print(f"旧任务总数: {sum(len(v) for v in old.values())}  (唯一 sut_function: {len(old_names)})")
print(f"新任务总数: {sum(len(v) for v in new.values())}  (唯一 sut_function: {len(new_names)})")
print(f"共有 sut_function: {len(common)}")

def dup_report(d, label):
    dups = {k: len(v) for k, v in d.items() if len(v) > 1}
    if dups:
        print(f"[{label}] 重复 sut_function {len(dups)} 个: {dict(list(dups.items())[:5])}")
dup_report(old, "旧")
dup_report(new, "新")

print(f"\n=== 仅旧有 (新生成时丢失): {len(only_old)} 个 ===")
for n in only_old[:30]:
    print(f"  - {n}")
if len(only_old) > 30:
    print(f"  ... 共 {len(only_old)} 个")

print(f"\n=== 仅新有 (新增): {len(only_new)} 个 ===")
for n in only_new[:30]:
    print(f"  + {n}")
if len(only_new) > 30:
    print(f"  ... 共 {len(only_new)} 个")

field_diff = Counter()
field_examples = defaultdict(list)
oracle_same = oracle_raw_diff = oracle_norm_diff = oracle_real_diff = 0
oracle_missing = 0
oracle_diff_samples = []
oracle_ws_samples = []
src_path_diff_names = []
masked_diff_names = []
real_oracle_diff_names = []

for name in sorted(common):
    o_tasks, n_tasks = old[name], new[name]
    if len(o_tasks) != 1 or len(n_tasks) != 1:
        field_diff["(sut_function 出现多次)"] += 1
        continue
    o, n = o_tasks[0], n_tasks[0]

    for f in COMPARE_FIELDS:
        if o.get(f) != n.get(f):
            field_diff[f] += 1
            if f == "source_path":
                src_path_diff_names.append(name)
            if f == "masked_code":
                masked_diff_names.append(name)
            if len(field_examples[f]) < 3:
                field_examples[f].append((name, o.get(f), n.get(f)))

    oo = oracle_text(o, "oracles-normal")
    no = oracle_text(n, "oracles")
    if oo is None or no is None:
        oracle_missing += 1
    elif oo == no:
        oracle_same += 1
    else:
        oracle_raw_diff += 1
        if norm(oo) == norm(no):
            oracle_norm_diff += 1
            if len(oracle_ws_samples) < 2:
                oracle_ws_samples.append((name, oo, no))
        else:
            oracle_real_diff += 1
            real_oracle_diff_names.append(name)
            if len(oracle_diff_samples) < 8:
                oracle_diff_samples.append((name, o.get("oracle"), n.get("oracle"), oo, no))

print(f"\n=== 共有 {len(common)} 个 sut_function 的字段差异统计 ===")
for f, cnt in field_diff.most_common():
    print(f"  {f:40s} 不同: {cnt:4d}")
print(f"  oracle 完全相同: {oracle_same}  |  原始内容不同: {oracle_raw_diff}  "
      f"(仅空白/换行差异: {oracle_norm_diff} + 真实内容差异: {oracle_real_diff})  |  缺失: {oracle_missing}")

if field_diff:
    print("\n=== 字段差异示例 (sut_function | 旧值 | 新值) ===")
    for f, cnt in field_diff.items():
        if f == "(sut_function 出现多次)":
            continue
        print(f"\n--- 字段: {f} (不同 {cnt} 个) ---")
        for name, ov, nv in field_examples[f]:
            ovs = (ov or "")[:120].replace("\n", "\\n")
            nvs = (nv or "")[:120].replace("\n", "\\n")
            print(f"  {name}:\n    旧: {ovs}\n    新: {nvs}")

if oracle_ws_samples:
    print("\n=== 仅空白差异的样例 (查看具体差在哪) ===")
    for name, oo, no in oracle_ws_samples[:2]:
        print(f"\n--- {name} ---")
        print(f"    旧 repr 末 60 字符: {repr(oo[-60:])}")
        print(f"    新 repr 末 60 字符: {repr(no[-60:])}")

if oracle_diff_samples:
    print(f"\n=== oracle 真实内容差异 (normalized 后不同, 共 {len(oracle_diff_samples)}+ 个, 列前 {len(oracle_diff_samples)}) ===")
    for name, o_or, n_or, oo, no in oracle_diff_samples:
        print(f"\n--- {name}  旧={o_or}  新={n_or} ---")
        diff = list(difflib.unified_diff(oo.splitlines(), no.splitlines(),
                                         "old", "new", lineterm=""))
        for line in diff[:30]:
            print(f"    {line}")
        if len(diff) > 30:
            print(f"    ... 共 {len(diff)} 行 diff")

if src_path_diff_names:
    print(f"\n=== source_path 不同的函数名单 ({len(src_path_diff_names)}) ===")
    print("  " + ", ".join(src_path_diff_names))
if masked_diff_names:
    print(f"\n=== masked_code 不同的函数名单 ({len(masked_diff_names)}) ===")
    print("  " + ", ".join(masked_diff_names))
if real_oracle_diff_names:
    print(f"\n=== oracle 真实内容不同的函数名单 ({len(real_oracle_diff_names)}) ===")
    print("  " + ", ".join(real_oracle_diff_names))

print("\n=== 结论 ===")
n_field_only = len([f for f in field_diff if f != "(sut_function 出现多次)"])
all_same = (len(only_old) == 0 and len(only_new) == 0 and n_field_only == 0
            and oracle_raw_diff == 0 and oracle_missing == 0)
if all_same:
    print("旧数据集与新数据集的样本数据(除编号外)完全一致。")
else:
    print(f"存在差异: 仅旧 {len(only_old)} / 仅新 {len(only_new)} / 字段不同 {n_field_only} "
          f"/ oracle 原始不同 {oracle_raw_diff} (仅空白差异 {oracle_norm_diff}) / oracle 缺失 {oracle_missing}")
