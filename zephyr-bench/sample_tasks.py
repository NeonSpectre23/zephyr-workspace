#!/usr/bin/env python3
"""按 module(L1)分层抽取一个任务子集,给 runner --data 用(省 token / 保证模块覆盖)。

模块划分复用 zephyr-claude/whitebox.py 的 classify_module(顶层目录 + include 归并)。
输出仍为 jsonl(每条任务记录原样),可直接:
  cd zephyr-claude
  python runner.py --data ../zephyr-bench/zephyr_tasks.sample.jsonl --batch 100

用法:
  python sample_tasks.py --total 100 --strategy proportional_min --seed 0 \
        --out zephyr_tasks.sample.jsonl
策略:
  equal            每模块均分(按模块数四舍五入,受各模块总量封顶)
  proportional     按模块占比取 floor,余数按最大余数分配
  proportional_min 每非空模块至少 1 条,其余按占比分配(推荐)
可选 --module 过滤(可多传),--print-ids 打印选中 task_id。
"""
import argparse
import json
import random
import sys
from collections import Counter, defaultdict
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "zephyr-claude"))
from whitebox import classify_module  # noqa: E402

BENCH = Path(__file__).resolve().parent


def load_all():
    tasks = []
    for f in ("zephyr_tasks.c.jsonl", "zephyr_tasks.h.jsonl"):
        p = BENCH / f
        if p.exists():
            for line in open(p, encoding="utf-8"):
                if line.strip():
                    tasks.append(json.loads(line))
    return tasks


def _allocate(total, avail: dict, strategy: str) -> dict:
    """返回 module -> 计划抽取数(未超各 module 总量)。"""
    mods = sorted(avail)  # 稳定顺序
    n_mod = len(mods)
    if total >= sum(avail.values()):
        return dict(avail)  # 全取

    if strategy == "equal":
        base = {m: max(1, total // n_mod) for m in mods}
        plan = {m: min(avail[m], base[m]) for m in mods}
    elif strategy == "proportional":
        total_avail = sum(avail.values())
        base = {m: avail[m] * total // total_avail for m in mods}
        plan = {m: min(avail[m], base[m]) for m in mods}
    else:  # proportional_min
        plan = {m: 1 for m in mods}          # 每模块至少 1
        rest = sum(avail.values()) - len(mods)   # 去掉保底后可用
        budget = total - len(mods)
        if budget > 0 and rest > 0:
            frac = {m: (avail[m] - 1) * budget / rest for m in mods}
            plan = {m: 1 + int(frac[m]) for m in mods}
            remain = total - sum(plan.values())
            order = sorted(mods, key=lambda m: frac[m] - int(frac[m]), reverse=True)
            for m in order:
                if remain <= 0:
                    break
                if plan[m] < avail[m]:
                    plan[m] += 1
                    remain -= 1
        elif budget <= 0:
            # total 小于模块数:优先取占比大的模块各 1 条
            plan = {m: 1 for m in sorted(mods, key=lambda m: -avail[m])[:max(total, 0)]}

    # 最后统一 clamp 到各自总量
    plan = {m: min(avail[m], plan.get(m, 0)) for m in mods}
    # 若因 clamp 有富余,再按占比补足到 total
    while sum(plan.values()) < total:
        cand = [m for m in mods if plan[m] < avail[m]]
        if not cand:
            break
        m = max(cand, key=lambda m: avail[m])  # 从最大模块补
        plan[m] += 1
    return {m: v for m, v in plan.items() if v > 0}


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--total", type=int, default=100)
    ap.add_argument("--strategy", choices=["equal", "proportional", "proportional_min"],
                    default="proportional_min")
    ap.add_argument("--seed", type=int, default=0)
    ap.add_argument("--out", default=str(BENCH / "zephyr_tasks.sample.jsonl"))
    ap.add_argument("--module", action="append", help="只保留这些 module(可多传)")
    ap.add_argument("--print-ids", action="store_true")
    a = ap.parse_args()

    rng = random.Random(a.seed)
    tasks = load_all()
    grouped = defaultdict(list)
    for t in tasks:
        m = classify_module(t.get("source_path", ""))
        grouped[m].append(t)
    avail = {m: len(ts) for m, ts in grouped.items()}
    if a.module:
        avail = {m: avail[m] for m in a.module if m in avail}

    plan = _allocate(a.total, avail, a.strategy)
    picked = []
    for m, n in plan.items():
        picked.extend(rng.sample(grouped[m], n))
    # 默认按 task_id 升序输出,便于对照 results 与复现;seed 只影响"抽了哪些"
    picked.sort(key=lambda t: int(t["task_id"]))

    with open(a.out, "w", encoding="utf-8") as f:
        for t in picked:
            f.write(json.dumps(t, ensure_ascii=False) + "\n")

    dist = Counter(classify_module(t["source_path"]) for t in picked)
    print(f"选中 {len(picked)} 条 → {a.out}")
    print("module 分布:", dict(sorted(dist.items(), key=lambda x: -x[1])))
    if a.print_ids:
        print("task_ids:", ",".join(t["task_id"] for t in picked))


if __name__ == "__main__":
    main()
