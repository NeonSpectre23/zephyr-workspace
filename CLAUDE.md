# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## 这是什么

**Zephyr RTOS C 函数补全基准**：把 Zephyr 源码中某个函数的函数体替换为 `/* MASKED */`，让被测 LLM（通过 Docker 里的 Claude Code CLI 跑）补全，再把模型输出注入真实源码，用 `west build` + 真实 ZTEST 单元测试判定通过/失败。从 RIOT-OS 基准移植、适配 Zephyr 4.4.0。代码注释与文档基本为中文。

- **zephyr-bench/** — 数据集模块（712 个已验证任务 + oracle 参考实现 + 构建脚本），在 git 中。
- **zephyr-claude/** — 评估框架（agent 流水线 + 沙箱 + 质量验证 + 失败分析），在 git 中。
- **zephyr/ modules/ bootloader/ tools/** — Zephyr RTOS 源码及模块，**不在 git**，由 `west init/update` 拉取（见 `.gitignore`）。

## 关键架构

### 评测流水线（zephyr-claude/agent.py `ClaudeAgent.run`）

每个 task 依序执行，`finally` 一定做清理：

1. `hard_reset_repo(zephyr/, clean=True)` 恢复 Zephyr 源码树。
2. **只复制目标文件**到 `WORKSPACE_BASE/task_N/target.c`（默认 `~/.zephyr-workspaces`），`apply_mask` 遮蔽函数体，写入 `claude_home/settings.json`（追加 `claude-on-completion` 钩子打印 `=== ZEPHYR_TASK_COMPLETED ===`）。
3. `_capture_snapshot`：git HEAD、docker 镜像 id/digest、模型名、prompt/数据集哈希等。
4. `_run_claude`：Docker 跑 Claude Code CLI。整个 `zephyr/` **只读挂载** `/workspace:ro`，仅目标文件在 `/workspace/<source_path>` 可写；`--cap-drop ALL`、`--disallowedTools "WebSearch,WebFetch"` 防联网；无输出超过 `WATCHDOG_TIMEOUT`(8h) 则 kill。结果以 `claude_completed` 标记。
5. 读回 `target.c`，对 masked vs final 做 unified diff。
6. **完整性检查**（见下）→ 违规判 `illegal_modifications`。
7. `_verify_tests`：把模型代码写回真实 `zephyr/` 源码 → 跑 `task["run_command"]`（west build + 跑测试）→ **立即恢复原文件** → 解析输出。
8. 追加轨迹日志 `zephyr-claude/trajectory/task_N.log`、结果到 `results/results.jsonl`；清理 workspace + git reset。

**判定规则**（agent.py `_verify_tests`）：通过 = 无编译错（`BUILD FAILED|FATAL ERROR|ninja: error|undefined reference`）+ 无崩溃（`Segmentation fault|Aborted|panic`）+ 无目标 `FAIL - <unit_test>` + 出现 `PROJECT EXECUTION SUCCESSFUL`。错误类别见 agent.py 顶部常量（`compile_error` / `crash` / `test_failure` / `test_not_executed` / `timeout` / `watchdog` / `api_error` / `illegal_modifications` / `none`）。

### 防作弊/完整性检查（zephyr-claude/mask.py）

- `check_ast_integrity`：对 masked 与 final 两版分别做**归一化**（函数体→占位符、参数名→pN、签名空白压平）再比较——发现目标函数之外的改动。
- `check_body_integrity`：函数体内禁止 `#define/#undef/#include/typedef/__asm__/_Generic`。
- 全部 tree-sitter 字节偏移都经 `_char_offset()` 转 Python 字符偏移（多字节 UTF-8 源文件必须如此），写 AST 编辑代码时沿用此模式。

### 数据集质量验证（两轮判定）

任务"可用"标准：oracle 注入必须 PASS + 空实现(stub)注入必须 FAIL。

- `zephyr-claude/inject_test.py`：注入 oracle → 跑测试（验证数据集"能过"）。
- `zephyr-claude/neg_control.py`：注入可编译空实现 → 跑测试，判定 `GOOD`（oracle PASS 且 stub FAIL）/ `WEAK`（两者都 PASS，测试没真验证函数，弃）/ `ORACLE_FAIL` / `ORACLE_SKIP` / `NEG_COMPILE_FAIL`。输出 `neg_control_results.json` + 进度日志。
- `zephyr-bench/filter_dataset.py`：只保留 `GOOD`，**.c 重编号 1..N，.h 从 N+1 续编**，重建 `oracles/`，原数据备份为 `*-full.jsonl` / `oracles-full/`。当前最终 712 = 493(.c) + 219(.h)，对应 `oracles/` 712 个 `<id>.{c,h}` 文件。

### 数据集构建流水线（zephyr-bench/，改动数据集时）

`extract_zephyr_tests.py`（扫 ZTEST/ZTEST_F 块 → `zephyr_tests.jsonl`）→ `build_zephyr_dataset.py`（tree-sitter 解析测试里的 call_expression 找 SUT → 按目录优先级 `lib > subsys > kernel/arch > drivers` 定位定义 → 生成 masked_code + oracles → `zephyr_tasks.c/.h.jsonl`）→ `verify_zephyr_tests.py`（west build 或 `--lightweight` GCC 两种模式）→ 人工跑 neg_control/inject_test → `filter_dataset.py`。

每任务 JSONL 字段：`task_id`、`source_path`、`sut_function`、`oracle`、`masked_code`（签名 + `/* TODO(agent) */`）、`unit_test`、`run_command`、`suite`、`zephyr_test_module`。排除 `tests/build/doc/scripts/boards/soc` 目录与框架函数（`zassert_*`、`printk`、`memcpy` 等）。

### 结果与分析脚本

- 评测结果写 `zephyr-claude/results/results.jsonl`（每行一条，见各 README 的 schema），轨迹在 `zephyr-claude/trajectory/`。分支上也保留过一次分模型运行的独立产物：`results-deepseek-v4-pro/` + `trajectory-deepseek-v4-pro/`（`runner.py` 的固定输出目录仍是 `results/` + `trajectory/`）。
- `whitebox.py`：把失败分类成对齐 QSemOS/RIOT 的 L2 taxonomy + mechanism（hallucination / misunderstanding / false_failure），输出 `whitebox_report.json`；指令遵循失败不进 whitebox。
- `analyze_run.py`：whitebox 复用流水线（`classify` 剔假失败+修正 L1 / `casebook` / `prelabel` / `finalize` / `normalize` 重写 results / `audit` 口径自查 / `selftest`），多模型（GLM）跑同一套口径，详见 zephyr-claude README。
- `annotate_worklist.py`：失败样本 casebook 生成（oracle-vs-Claude 对照），供逐条人工/AI 判 L2/L3。
- （旧 `analyze_results.py`/`final_analysis.py` 已归档至 `_trash/legacy_analysis/`——被 `whitebox.py`+`analyze_run.py` 取代。）

## 常用命令

环境激活（conda env `zephyr` + `ZEPHYR_BASE`/`ZEPHYR_SDK_INSTALL_DIR`）：
```bash
source activate.sh
```

依赖自检 / 跑评测 / 续跑：
```bash
cd zephyr-claude
python runner.py --check                 # 逐项自检 Docker/镜像/tree-sitter/settings/zephyr/SDK/数据集
python runner.py --task-id 1             # 单任务
python runner.py --batch 10              # 批量前 N 个
python runner.py --batch 100 --resume    # 断点续跑：跳过 results.jsonl 里已 passed 的任务
python runner.py --batch 50 --start 200  # 从某 task_id 起
python runner.py --data <file.jsonl>     # 自定义数据集
```

数据集质量验证与筛选（需 Zephyr SDK + 工具链，慢）：
```bash
python inject_test.py --batch 5                      # ① oracle 注入应通过
python neg_control.py --batch 5                      # ② 空实现应失败；--resume 续跑
python neg_control.py --recheck NEG_COMPILE_FAIL ... # 只重跑某判定
python ../zephyr-bench/filter_dataset.py --orig final_verdicts.json --bench ../zephyr-bench
```

分析：
```bash
python whitebox.py                   # → results/whitebox_report.json（需先逐条人工/AI 填 L2/L3 或用 finalize 合并 annotations）
python analyze_run.py selftest       # 分类回归自检
# analyze_run.py classify/casebook/prelabel/finalize 见 zephyr-claude README
```

Lint（CI 只跑 ruff，无 pytest 套件；CI 的 verify-dataset job 是纯 Python 结构校验）：
```bash
ruff check zephyr-claude/            # ruff.toml 已在仓库根
```

## 约定与易错点

- **`zephyr/` 是个独立 git 仓库（west snapshot）**。任务前/后都要 `git reset --hard`，假设工作树干净；不要在 `zephyr/` 里留下任何改动或提交。`git_utils.hard_reset_repo` 的 `git clean -fd` 会排除 `zephyr-claude/ zephyr-bench/ .claude_home/ build/`。
- **`from config import *` 是有意为之**（agent.py / runner.py / config.py 大量用 config 的模块级常量）。`ruff.toml` 已对这三个文件忽略 F403/F405——不要"修复"成显式 import。
- **API 配置查找顺序**（config.py `_default_claude_settings`）：`zephyr-claude/settings.json`（benchmark 专用，推荐，已被 gitignore）→ `~/.claude/settings.json` → WSL 里 `/mnt/c/Users/*/.claude/settings.json`；`ZEPHYR_CLAUDE_SETTINGS` 可强制指定。配置内容为 `ANTHROPIC_BASE_URL` / `ANTHROPIC_AUTH_TOKEN` / `ANTHROPIC_MODEL` 等 env，模板见 `claude_settings.template`。改模型/Key 只改配置文件即可，`agent.py` 会注入容器。
- **环境变量覆盖**：`ZEPHYR_SDK_INSTALL_DIR`、`ZEPHYR_CLAUDE_SETTINGS`、`ZEPHYR_CLAUDE_NETWORK`（默认 `host`，改模型必须能连网）、`ZEPHYR_WORKSPACE_BASE`、`ZEPHYR_CLAUDE_RUN_NAME`（多模型跑：输出到 `results-<name>/` + `trajectory-<name>/`）、`ZEPHYR_CLAUDE_IDLE_TIMEOUT`（空闲保险丝秒数，默认 1800：Claude 连续无新输出则 kill，防 API 流静默挂起）。
- `west build` 验证需要完整 Zephyr SDK + 工具链，应在 WSL 内跑；不要用 `--lightweight` 之外的模式指望在 CI 里验证。
- 顶层 `setup.sh` 一键装环境；`zephyr-bench/` 与 `zephyr-claude/` 各自的 README 是最新的模块级文档。
