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
| `analyze_run.py` | **whitebox 复用流水线**：classify（剔假失败+修正 L1）/ casebook / prelabel / finalize / selftest |
| `annotate_worklist.py` | 失败样本 casebook 生成（oracle-vs-Claude 对照，供逐条人工/AI 判 L2/L3） |
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

## 构建 Docker 沙箱

评测在 Docker 容器内运行 Claude Code CLI，需先构建沙箱镜像：

```bash
cd zephyr-claude
docker build -t zephyr-sandbox:latest .
```

> 沙箱容器以 `--network host` 启动（`ZEPHYR_CLAUDE_NETWORK`，默认 `host`）——这是必需的，因为容器内的 Claude Code 要调用 LLM API（如 `api.deepseek.com`）。如需临时改用其他网络（如 `bridge`）：`export ZEPHYR_CLAUDE_NETWORK=bridge`。同时启动命令带 `--disallowedTools "WebSearch,WebFetch"`，禁用 Claude 的 web 搜索/抓取工具，防止被测模型联网查答案。注意这只禁用了 web 工具，容器内 `Bash` 未禁用。

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

### whitebox 复用流水线（analyze_run.py）

一次模型 run（`results*.jsonl` + `trajectory*/`）的确定性分析入口，供多模型（如 GLM）复用同一套口径：

```bash
python analyze_run.py classify  --results results-<m>/results.jsonl --trajectory trajectory-<m>
#   → 剔除 illegal / harness 假失败(FF) / "实过却判败"(日志有 PROJECT EXECUTION SUCCESSFUL),
#     对每条真实失败给出修正 L1(compile_error / crash / test_failure)
python analyze_run.py casebook  --results ... --trajectory ... --out casebook.jsonl
#   → 拼出每条 oracle-vs-Claude 对照(供逐条判 L2/L3)
python analyze_run.py prelabel  --casebook casebook.jsonl --out prelabel.jsonl
#   → 确定性预标(只有把握的模式 + confidence;其余 needs_llm)
python analyze_run.py finalize  --annotations annotations.jsonl --casebook casebook.jsonl --out whitebox_report.json
#   → annotations(task_id/L1/L2/L3)→ canonical L2 + mechanism + module → 最终报告
python analyze_run.py normalize --results results-<m>/results.jsonl --trajectory trajectory-<m>
#   → 用修复后判定重写 results 的 passed/error_category(留 .raw 备份,零算力)
python analyze_run.py audit     --results results-<m>/results.jsonl --trajectory trajectory-<m> \
    --whitebox results-<m>/whitebox_report.json --out results-<m>/schema_audit.json
#   → 口径自查:illegal∩whitebox=∅ / 假失败剔除 / mechanism==L2_TO_MECH / 无占位 L2
python analyze_run.py selftest   # 分类逻辑回归自检
```

约定与白盒口径一致：**细粒度 L2/L3 需人工/AI 逐条判定**（参考 QSem/RIOT 也是 per-task 标注；`prelabel` 只自动标高置信子集，如编译期幻觉 `undefined_reference`/`hallucinated_api`、crash 空指针保护缺失候选），L1/mechanism/module 与报告生成全自动可复现。`agent.py` 判定已加固：`PROJECT EXECUTION SUCCESSFUL` 最优先(实过不算失败)、受控 panic 测试(Caught system error)不算崩溃、编译错只认"有编译错误且无测试运行"、test_failure 不依赖具体 `unit_test` 名。

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

> ⚠️ **口径说明**：早期 runner 结果里 `test_not_executed`/`compile_error` 有误标（只查特定 `unit_test` 名、`FATAL ERROR`/`panic` 单词误匹配、受控 panic 测试误判）。agent.py 判定已加固；分析历史结果时用 `analyze_run.py classify` 得到修正 L1，并把"实过却判败"样本剔除（见 `results-deepseek-v4-pro/annotation/excluded_false_failures.jsonl`）。

## 配置

### API 配置（settings.json）

基准测试在 Docker 容器里运行 Claude Code CLI，需要配置 LLM 的 API 地址和 Key。配置文件按以下优先级查找（见 `config.py` 的 `_default_claude_settings()`），也可用环境变量 `ZEPHYR_CLAUDE_SETTINGS` 强制指定任意配置文件：

1. **`zephyr-claude/settings.json`** — benchmark 专用配置，**推荐使用**。它不在任何 `.claude/` 目录下，交互式 Claude 根本不会读取，因此改基准测试的模型/Key 完全不影响你日常使用（例如 cc-switch 的配置）；
2. **`~/.claude/settings.json`** — 你的交互式 Claude 配置，作为回退项；
3. WSL 下另会扫描 `/mnt/c/Users/*/.claude/settings.json`。

配置方法（benchmark 专用）：

```bash
cd zephyr-workspace
cp claude_settings.template zephyr-claude/settings.json
# 编辑 zephyr-claude/settings.json → 把 "sk-xxx" 替换成真实 Key
```

该文件已被 `.gitignore` 排除（`**/settings.json`），不会提交到仓库。支持的 API 提供方（通过 `ANTHROPIC_BASE_URL` 配置）：DeepSeek `https://api.deepseek.com/anthropic`、Anthropic `https://api.anthropic.com`、任意 Anthropic 兼容代理。默认模型 `deepseek-v4-flash`——按需修改 `ANTHROPIC_MODEL`（连同 `ANTHROPIC_DEFAULT_*_MODEL` 一起改）。

> 每次跑任务时 `agent.py` 会把选中的配置文件复制进容器内 Claude 的 `claude_home/settings.json`，并追加一个 `claude-on-completion` 钩子用于检测任务完成，同时把其中 `ANTHROPIC_*` 环境变量通过 `-e` 传入容器。改模型/Key 只需改配置文件，不用改代码。

### 环境变量

| 变量 | 作用 |
|---|---|
| `ZEPHYR_CLAUDE_SETTINGS` | 指定 API 配置文件 |
| `ZEPHYR_CLAUDE_NETWORK` | Docker 网络模式（默认 `host`） |
| `ZEPHYR_SDK_INSTALL_DIR` | Zephyr SDK 路径（默认 `$HOME/zephyr-sdk-1.0.1`） |
| `ZEPHYR_WORKSPACE_BASE` | 任务工作区目录（默认 `$HOME/.zephyr-workspaces`） |
| `ZEPHYR_CLAUDE_RUN_NAME` | 运行名（如 `glm-5.1`）：输出到 `results-<name>/` + `trajectory-<name>/`，多模型跑互不覆盖 |
| `ZEPHYR_CLAUDE_IDLE_TIMEOUT` | 空闲保险丝（秒，默认 1800=30 分钟）：容器内 Claude 连续无新输出则 kill（防 API 流静默挂起空等 8h） |

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

## 口径与差异说明（对照整改清单）

- **数据集 712 vs 885**：本 track 最终用 712 个经两轮验证的 GOOD 任务；清单 §二 stage-2 提到的 885 是 PR#3 原始集（未过滤弱任务）。如需 885，可用 `neg_control` + `filter_dataset.py` 重建/再筛。
- **网络模式**：`config.py` 默认 `--network host`（与既有 track 的 `none` **不同**，属清单硬伤②）——原因是容器内 Claude Code CLI 需访问 LLM API；但是对齐了RIOT和Qsem中的处理方法。
- **结果目录**：多模型用 `results-<run>/` + `trajectory-<run>/`（`ZEPHYR_CLAUDE_RUN_NAME`）。当前全量产物：`results-deepseek-v4-pro/`（712，whitebox 86 条）与 `results-glm/`（712，whitebox 27 条），各含 `annotation/`（casebook + annotations）与 `schema_audit.json`。
- **通过率口径（约定）**：分母固定为 **712**；`illegal_modifications` 与 harness 层失败（`timeout`/`watchdog`/`api_error`/`exception`，即 FF）**均计入分母、按失败计**（保守口径，两模型同口径）。当前：deepseek 612/712（86.0%）、glm 669/712（94.0%）。
- **机制统计口径**：机制/白盒统计**剔除 FF 与 illegal**（清单铁律①：`is_false_failure` 整条剔除、不计入失败；instruction-following 只由 `results.jsonl` 的 `illegal_changes` 提供、与白盒不相交）。即"通过率"与"机制统计"的分母不同，需在论文中分别注明。
- **口径自查**：`analyze_run.py audit` 断言 `illegal ∩ whitebox = ∅`、白盒=判定出的真实失败集、假失败已剔、`mechanism == L2_TO_MECH[L2]`、无占位 L2；两模型均 `all_pass: true`。`analyze_run.py normalize` 用修复后判定重写 `results` 的 `passed/error_category`（留 `.raw` 备份，零算力）。
- **schema/口径**：`whitebox_report.json` 字段与 QSem/RIOT 参考一致；mechanism 三分类由权威 `L2_TO_MECH` 派生；`instruction_following` 仅由 `results.jsonl` 的 `illegal_changes` 提供且与白盒不相交。

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
