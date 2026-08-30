#!/usr/bin/env python3
"""从 neg_control 结果中只保留 GOOD 任务，重编号并重新生成 oracles，输出最终数据集。

用法：
  python filter_dataset.py \
      --orig zephyr-claude/neg_control_results_1.json \
      --recheck zephyr-claude/neg_control_recheck.json
      [--bench zephyr-bench] [--no-backup]

- 判定合并：--recheck 中的任务以 recheck 为准，其余用 --orig
- 只保留 verdict == GOOD 的任务
- 重新编号：.c 任务 1..N_c，.h 任务从 N_c+1 起（保持全局唯一）
- oracle 内容从旧文件复制到新编号，任务记录里的 oracle 路径同步更新
- 原数据集备份为 *-full.jsonl 和 oracles-full/
"""
import argparse
import json
import shutil
from pathlib import Path


def main():
    parser = argparse.ArgumentParser(description="Filter dataset to GOOD tasks only")
    parser.add_argument("--orig", required=True, help="原始全量 neg_control 结果 JSON")
    parser.add_argument("--recheck", default="", help="recheck 结果 JSON（可选，覆盖对应任务判定）")
    parser.add_argument("--bench", default=".", help="zephyr-bench 目录（默认当前目录）")
    parser.add_argument("--no-backup", action="store_true", help="不备份原数据集")
    args = parser.parse_args()

    bench = Path(args.bench).resolve()

    # ---- 1. 合并判定 ----
    merged = json.load(open(args.orig, encoding="utf-8"))
    if args.recheck:
        recheck = json.load(open(args.recheck, encoding="utf-8"))
        for tid, v in recheck.items():
            merged[str(tid)] = v
    good = {str(tid) for tid, v in merged.items() if v.get("verdict") == "GOOD"}
    print(f"判定合并完成: 总数 {len(merged)}, GOOD {len(good)}")

    # ---- 2. 备份原数据集 ----
    if not args.no_backup:
        for name in ["zephyr_tasks.c.jsonl", "zephyr_tasks.h.jsonl"]:
            src = bench / name
            if src.exists():
                shutil.copy2(src, bench / name.replace(".jsonl", "-full.jsonl"))
        src_oracles = bench / "oracles"
        if src_oracles.is_dir():
            bak = bench / "oracles-full"
            if bak.exists():
                shutil.rmtree(bak)
            shutil.copytree(src_oracles, bak)
        print(f"已备份原数据集到 *-full.jsonl 和 oracles-full/")

    # ---- 3. 加载任务并筛选 ----
    def load(path):
        tasks = []
        for line in open(path, encoding="utf-8"):
            line = line.strip()
            if line:
                tasks.append(json.loads(line))
        return tasks

    c_tasks = [t for t in load(bench / "zephyr_tasks.c.jsonl") if t["task_id"] in good]
    h_tasks = [t for t in load(bench / "zephyr_tasks.h.jsonl") if t["task_id"] in good]
    print(f"保留: .c {len(c_tasks)} 个, .h {len(h_tasks)} 个")

    # ---- 4. 重建 oracles ----
    # 先把当前 oracles/ 里所有需要的 oracle 内容读进内存，再清空重建。
    # 注意：本次任务的编号是上一轮重编号后的（非 oracles-full 的 884 编号），
    # 所以必须从当前 oracles/ 按任务自己的 oracle 路径读，不能依赖 oracles-full。
    out_oracles = bench / "oracles"
    oracle_cache = {}
    for task in c_tasks + h_tasks:
        src = bench / task["oracle"]
        if src.exists():
            oracle_cache[task["task_id"]] = src.read_bytes()
        else:
            print(f"  WARNING: oracle 缺失 {src}（task {task['task_id']}）")
    for f in out_oracles.glob("*"):
        if f.is_file():
            f.unlink()

    def write_tasks(tasks, suffix, start_id, path):
        with open(path, "w", encoding="utf-8") as f:
            for i, task in enumerate(tasks, start=1):
                new_id = start_id + i - 1
                new_name = f"{new_id}.{suffix}"
                new_path = out_oracles / new_name
                content = oracle_cache.get(task["task_id"])
                if content is None:
                    print(f"  WARNING: 无 oracle 内容（task {task['task_id']}）")
                    continue
                new_path.write_bytes(content)
                record = dict(task)
                record["task_id"] = str(new_id)
                record["oracle"] = f"oracles/{new_name}"
                f.write(json.dumps(record, ensure_ascii=False) + "\n")

    write_tasks(c_tasks, "c", 1, bench / "zephyr_tasks.c.jsonl")
    write_tasks(h_tasks, "h", len(c_tasks) + 1, bench / "zephyr_tasks.h.jsonl")

    n_oracles = len(list(out_oracles.glob("*")))
    print(f"最终数据集: {len(c_tasks)} .c + {len(h_tasks)} .h = {len(c_tasks) + len(h_tasks)} 任务")
    print(f"oracles 重建: {n_oracles} 个")


if __name__ == "__main__":
    main()
