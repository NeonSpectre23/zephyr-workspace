# zephyr-claude — Zephyr 代码补全评估框架

本目录是 **Zephyr RTOS 代码补全基准**的评估框架：读取 `zephyr-bench/` 的最终数据集（712 个已验证任务），把目标函数体遮蔽为 `/* MASKED */`，在 Docker 沙箱里调用 Claude Code CLI 让被测模型补全，再把模型输出注入真实 Zephyr 源码，通过 `west build` + 运行单元测试判定通过/失败；并配套**数据集质量验证**与**失败机理分析**工具。

> 适配自 riot-claude。数据集见 [`../zephyr-bench/README.md`](../zephyr-bench/README.md)。

## 目录结构

| 文件 | 职责 |
|---|---|
| `runner.py` | **CLI 入口**：单任务/批量/断点续跑/依赖自检 |
| `agent.py` | **主流水线**：mask → Claude → diff → 完整性检查 → west build 验证 |
| `mask.py` | tree-sitter AST 操作：遮蔽、归一化等价性检查、函数体关键字检查 |
| `prompt.py` | 发给 Claude Code CLI 的任务提示词模板 |
| `config.py` | 全局配置：路径、Docker 网络/资源限额、数据集文件、API settings |
| `git_utils.py` | 任务前后 `git reset --hard` + `git clean` 恢复 Zephyr 源码 |
| `logger.py` | 轨迹日志追加写入 |
| `inject_test.py` | **答案注入验证**：注入 oracle → build → 测试，验证数据集"能过" |
| `neg_control.py` | **负控制验证**：注入空实现，找出测试未真正验证函数的弱任务 |
| `whitebox.py` | **失败机理分类**：从 results + trajectory 生成对齐三 track 的 whitebox_report |
| `analyze_results.py` | 结果统计（通过率/错误分类/运行时/按目录/按文件类型） |
| `final_analysis.py` | 失败 case 逐例复盘（读 trajectory） |
| `Dockerfile` | 沙箱镜像：Ubuntu 24.04 + west + Claude Code CLI |
| `settings.json` | benchmark 专用 API 配置（Key/模型，**已被 gitignore**） |

验证产物：`neg_control.log` / `neg_control_results.json`（最终 712 全量验证）、`final_verdicts.json`（筛选数据集用的合并判定）。

## 工作原理（agent.py 流水线）

```
① hard_reset_repo()           git reset --hard + clean，恢复 Zephyr 源码
② _prepare_workspace()        只复制目标源文件，apply_mask 把函数体换成 /* MASKED */
③ _capture_snapshot()         记录 git HEAD、镜像摘要、模型名、prompt 哈希、数据集哈希
④ _run_claude()               Docker 容器运行 Claude Code CLI 补全目标函数
⑤ 读 target.c + diff          masked 版本 vs 模型输出
⑥ 完整性检查                  AST 归一化等价性（只许改目标函数体）+ 禁用关键字检查
⑦ _verify_tests()             把模型代码注入真实源码，west build + 跑 ztest，解析 PASS/FAIL
⑧ 记录结果 + 清理             写入 results.jsonl / trajectory/，git reset 还原
```

### 沙箱隔离（agent.py）

- 容器参数：`--cap-drop ALL`、`--security-opt no-new-privileges`、`--pids-limit`、`--memory`/`--cpus` 限额
- 整个 `zephyr/` 源码树**只读挂载** `/workspace:ro`，仅目标文件 `rw`
- `--disallowedTools "WebSearch,WebFetch"` 禁用模型联网
- 网络模式默认 `host`（`config.py` 的 `CLAUDE_NETWORK_MODE`，环境变量 `ZEPHYR_CLAUDE_NETWORK` 覆盖）

### 完整性/防作弊检查（mask.py）

- **AST 归一化等价性** `check_ast_integrity`：masked 和 final 两个版本归一化后比较——发现目标函数之外的修改
- **函数体关键字检查** `check_body_integrity`：禁止函数体内出现 `#define`、`#include`、`typedef`、`__asm__` 等
- 违规 → `error_category = illegal_modifications`

### 结果判定（agent.py `_verify_tests`）

通过 = 无编译错误 + 无崩溃 + 目标 `unit_test` 无 FAIL + `PROJECT EXECUTION SUCCESSFUL`。

| error_category | 含义 |
|---|---|
| `none` | 通过 |
| `illegal_modifications` | 修改了目标函数之外 / 体内含禁用关键字 |
| `compile_error` | west build 失败 |
| `crash` | 运行崩溃（segfault/Aborted/panic） |
| `test_failure` | 编译过但测试失败 |
| `test_not_executed` | 测试没有真正执行 |
| `timeout` / `watchdog` | build 超时 / Claude 会话超时 |
| `api_error` | Claude 未完成（API 异常） |

## 使用

### 依赖自检

```bash
python runner.py --check
```

### 运行单任务 / 批量 / 续跑

```bash
python runner.py --task-id 1
python runner.py --batch 10
python runner.py --batch 100 --resume        # 跳过已通过任务
python runner.py --batch 50 --start 200      # 从指定任务开始
```

### 数据集质量验证（答案注入 + 负控制）

```bash
# ① 答案注入：oracle 应能通过测试
python inject_test.py --batch 20

# ② 负控制：空实现应让测试失败（找出"假验证"弱任务）
python neg_control.py --batch 20
```

两个脚本都支持 `--resume` 断点续跑。`neg_control.py` 还支持只重跑特定判定：

```bash
python neg_control.py --recheck NEG_COMPILE_FAIL \
    --results neg_control_results.json \
    --out neg_control_recheck.json \
    --log neg_control_recheck.log
```

### 失败机理分析（whitebox）

```bash
# 先跑评测（产生 results/results.jsonl + trajectory/），再：
python whitebox.py
# 输出 results/whitebox_report.json（对齐 QSem/RIOT 的统一 taxonomy）
```

## 数据集质量验证的完整流程

**两轮验证**定义"可用任务"：oracle 注入必须通过（正确答案有效）+ 空实现注入必须失败（测试真验证函数）。

**判定**：`GOOD`（可保留）/ `WEAK`（测试没验证函数，弃）/ `ORACLE_FAIL`（oracle 或关联缺陷，弃）/ `NEG_COMPILE_FAIL`（空实现没编译）/ `ORACLE_SKIP`（函数找不到）。

**最终数据集形成**：`neg_control` 扫描 → 按判定筛除 → `zephyr-bench/filter_dataset.py` 只留 GOOD → 重编号。

**验证历程与最终结果**（2026-08）：

| 阶段 | 任务数 | 说明 |
|---|---|---|
| 生成 | 884 | 原始数据集 |
| 首轮 neg_control | 884 | GOOD 689 / NCF 101 / WEAK 69 / ORACLE_FAIL 25——101 个 NCF 实为工具 bug 误报 |
| 修复 stub 后重跑 | 779 | NCF 重跑 → 90 GOOD + 11 WEAK |
| **修复 classify 后全量复验** | **712** | **712/712 全部 GOOD（100%）** |

**验证中发现并修复的工具缺陷**：

| 缺陷 | 影响 | 修复 |
|---|---|---|
| `build_stub_text` 漏 `source[:a]`，删掉文件头 | 101 个任务误判 NEG_COMPILE_FAIL | `source[:a] + sig + stub + ...` |
| 旧 stub 一律 `return (类型){0}`，对"返回 0=成功"API 误判 WEAK | `bt_enable` 等误判 | 指针→NULL、有符号→-1、无符号→0 |
| `run_test` 只返回 stdout 末尾 300 字符、不含 stderr | 编译错误漏判 | 返回完整 stdout+stderr |
| `classify` 用 `"error:"` 判编译失败，误匹配 ZTEST 断言消息 | 真 GOOD 误判 NCF | 先看 `PROJECT EXECUTION` 标记判断测试是否执行 |

## 失败机理分类（whitebox.py，对齐 Failure Taxonomy）

按统一 taxonomy 把失败任务分类，与 QSemOS/RIOT 三 track 可比：

- **L1** = runner 的 `error_category`（compile_error / crash / test_failure / false_failure_detection）
- **L2** = 细粒度子类（经 `L2_CONSOLIDATED` 合并，如 `undefined_reference` / `missing_conditional_guard` / `needs_manual_inspection`）
- **mechanism** = 三大类（`hallucination` 幻觉 / `misunderstanding` 理解偏差 / `false_failure` 假失败）
- **module** = Zephyr 模块 L1（`classify_module`：顶层目录 + `include/zephyr/<sub>/` 归并到子系统）
- **双标签**：每条失败同时带 mechanism + module 两个平行维度
- 指令遵循失败（`illegal_changes` 非空）不进 whitebox，在 results.jsonl 单独统计

## 配置

### API 配置（settings.json）

```bash
cp ../claude_settings.template settings.json   # 填入真实 Key/模型
```

agent.py 会把这个文件复制进容器内 Claude 的 `claude_home/settings.json` 并注入 `ANTHROPIC_*` 环境变量。

### 环境变量

| 变量 | 作用 |
|---|---|
| `ZEPHYR_CLAUDE_SETTINGS` | 指定 API 配置文件 |
| `ZEPHYR_CLAUDE_NETWORK` | Docker 网络模式（默认 `host`） |
| `ZEPHYR_SDK_INSTALL_DIR` | Zephyr SDK 路径（默认 `$HOME/zephyr-sdk-1.0.1`） |
| `ZEPHYR_WORKSPACE_BASE` | 任务工作区目录（默认 `$HOME/.zephyr-workspaces`） |

### 结果字段（results.jsonl）

```json
{
  "task_id": "1",
  "passed": true,
  "claude_completed": true,
  "runtime_s": 45.2,
  "illegal_changes": [],
  "error_category": "none",
  "patch": "...",
  "snapshot": { "git_head": "...", "claude_model": "...", "task": {...} }
}
```

## 环境依赖

| 依赖 | 说明 |
|---|---|
| **Docker** | 沙箱运行 Claude Code CLI，需先 `docker build -t zephyr-sandbox:latest .` |
| **tree-sitter** | `pip install tree-sitter tree-sitter-c`（mask.py 需要） |
| **Zephyr SDK** | `~/zephyr-sdk-1.0.1`，west build 需要 |
| **west + 工具链** | 验证阶段编译需要（在 WSL 里 `source ../activate.sh` 配置） |
| **Zephyr 源码** | `../zephyr`，west 拉取 |
| **settings.json** | 被测模型的 API Key/模型 |

> ⚠️ `west build` 验证与 `inject_test`/`neg_control` 都需要完整 Zephyr SDK + 工具链，应在 WSL 环境运行。`runner.py --check` 会逐项自检依赖。
