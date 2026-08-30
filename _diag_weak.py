#!/usr/bin/env python3
"""诊断:重编号后跑出的 WEAK 任务,映射回原始编号并对比原判定。"""
import json
import re
from pathlib import Path

b = Path("zephyr-bench")

def load_tasks(path):
    tasks = {}
    for line in open(path, encoding="utf-8"):
        line = line.strip()
        if line:
            t = json.loads(line)
            tasks[t["task_id"]] = t
    return tasks

new_c = load_tasks(b / "zephyr_tasks.c.jsonl")
new_h = load_tasks(b / "zephyr_tasks.h.jsonl")
old_c = load_tasks(b / "zephyr_tasks.c-full.jsonl")
old_h = load_tasks(b / "zephyr_tasks.h-full.jsonl")

# 新ID -> 旧ID (按 sut_function+source_path 匹配)
def build_map(new_tasks, old_tasks):
    old_by_key = {(t["sut_function"], t["source_path"]): tid for tid, t in old_tasks.items()}
    m = {}
    for tid, t in new_tasks.items():
        key = (t["sut_function"], t["source_path"])
        if key in old_by_key:
            m[tid] = old_by_key[key]
    return m

map_c = build_map(new_c, old_c)
map_h = build_map(new_h, old_h)
new2old = {**map_c, **map_h}

# 原判定
orig = json.load(open("zephyr-claude/neg_control_results_1.json"))

# 最近一次跑的 WEAK 任务(从 neg_control.log 最后 121 行取)
# 取日志中每个任务的最新判定
recent = {}
for line in open("zephyr-claude/neg_control.log", encoding="utf-8", errors="replace"):
    m = re.match(r"task (\S+): (\S+)", line)
    if m:
        recent[m.group(1)] = m.group(2)

weak_new = sorted([tid for tid, v in recent.items() if v == "WEAK"], key=int)
print(f"最近一次跑: {len(recent)} 个任务, WEAK {len(weak_new)} 个")
print()
print(f"{'新ID':>5} {'旧ID':>5}  {'原判定':<10} {'sut_function':<28} {'source_path'}")
for nid in weak_new:
    oid = new2old.get(nid, "?")
    orig_ver = orig.get(oid, {}).get("verdict", "?") if oid != "?" else "?"
    # 从新任务记录拿函数信息
    t = new_c.get(nid) or new_h.get(nid) or {}
    print(f"{nid:>5} {oid:>5}  {orig_ver:<10} {t.get('sut_function',''):<28} {t.get('source_path','')}")
