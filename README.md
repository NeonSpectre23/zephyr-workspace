# Zephyr RTOS 代码补全基准测试

用于评估大语言模型（Claude / DeepSeek 等）在 **Zephyr RTOS C 函数补全**任务上的表现——模型看到一份函数体被替换为 `/* MASKED */` 的 Zephyr 源文件，需要实现该函数，最终通过 `west build` 编译 + 运行真实单元测试来验证结果。

> 从 RIOT-OS 基准测试模式移植而来，适配 Zephyr 4.4.0，已通过 Anthropic 兼容 API 配合 DeepSeek v4 Flash 测试。
>
> **说明：** 本仓库只包含基准测试框架和数据集。Zephyr RTOS 源码及其模块**不包含在内**——它们会在安装时由 `west init` + `west update` 自动拉取。

## 仓库内容（git 内 vs. west 拉取）

| 包含在 git 中                                  | 由 `west update` 拉取         |
| ------------------------------------------ | -------------------------- |
| `zephyr-claude/` — 基准测试框架                  | `zephyr/` — Zephyr RTOS 源码 |
| `zephyr-bench/` — 712 个任务 + oracle         | `modules/` — HAL、加密、文件系统   |
| `activate.sh`、`setup.sh`、`environment.yml` | `bootloader/` — MCUboot    |
| `claude_settings.template`、`.gitignore`    | `tools/edtt/` — 蓝牙测试工具     |
| `README.md`、`.github/workflows/ci.yml`     | <br />                     |

### 目录结构

```
zephyr-workspace/
├── zephyr/               ← Zephyr RTOS v4.4.0（west 拉取，不在 git 中）
├── zephyr-bench/         ← 基准数据集（712 个已验证任务）+ 构建脚本  ★ 在 git 中
│   ├── zephyr_tasks.c.jsonl    493 个任务（C 文件）
│   ├── zephyr_tasks.h.jsonl    219 个任务（头文件）
│   ├── oracles/                712 个已验证的参考实现
│   ├── build_zephyr_dataset.py 从单元测试构建数据集
│   ├── extract_zephyr_tests.py ZTEST 块提取
│   ├── verify_zephyr_tests.py  Oracle 注入与验证
│   └── filter_dataset.py       按验证判定筛出最终数据集
├── zephyr-claude/        ← 评估框架  ★ 在 git 中
│   ├── agent.py               主流水线：mask → Claude → verify
│   ├── mask.py                tree-sitter AST 操作
│   ├── runner.py              CLI 入口
│   ├── prompt.py              LLM 提示词模板
│   ├── config.py              全局配置
│   ├── Dockerfile             沙箱镜像（Ubuntu + west + Claude Code CLI）
│   ├── inject_test.py         答案注入验证（oracle 应通过测试）
│   ├── neg_control.py         负控制验证（空实现应让测试失败）
│   ├── whitebox.py            失败机理分类（对齐三 track taxonomy）
│   ├── analyze_run.py         whitebox 复用流水线（classify/casebook/prelabel/finalize/normalize/audit/selftest）
│   ├── annotate_worklist.py   失败样本 casebook 生成（oracle-vs-Claude 对照）
│   └── settings.json          benchmark 专用 API 配置（gitignore，详见 zephyr-claude/README）
├── bootloader/           ← MCUboot（west 拉取，不在 git 中）
├── modules/              ← HAL、加密等（west 拉取，不在 git 中）
├── tools/                ←（west 拉取，不在 git 中）
├── activate.sh           ← 环境激活脚本
├── .gitignore
├── environment.yml       ← Conda 环境定义
├── claude_settings.template  ← Claude API 配置模板
├── setup.sh              ← 一键环境安装脚本
└── .github/workflows/    ← CI 配置
```

## 工作原理

```
1. git reset --hard HEAD          # 清理 Zephyr 源码
2. 复制目标文件到工作区            # 只保留要编辑的文件
3. tree-sitter：将函数体遮蔽为 /* MASKED */
4. 在 Docker 中运行 Claude Code CLI  # Zephyr 源码只读，目标文件可写
5. 读取 Claude 输出 → diff
6. AST 完整性检查                  # Claude 是否只修改了目标函数？
7. 将 Claude 代码注入原始源码
8. west build + 运行单元测试       # 能编译吗？测试通过吗？
9. git reset → 还原               # 恢复到干净状态
```

### 安全检查

- **AST 归一化相等性**：比较归一化后的遮蔽源码与最终源码——可发现目标函数之外的修改
- **函数体关键字检查**：禁止在函数体内出现 `#define`、`#include`、`typedef`、`__asm__`
- **Docker 沙箱**：`--cap-drop ALL`、`--security-opt no-new-privileges`、Zephyr 源码只读挂载、`--network host` + `--disallowedTools "WebSearch,WebFetch"`（禁用 web 工具）

## 环境要求

| 依赖                    | 版本 / 位置                                                                    | 说明                    |
| --------------------- | -------------------------------------------------------------------------- | --------------------- |
| **Zephyr SDK**        | [v1.0.1](https://github.com/zephyrproject-rtos/sdk-ng/releases/tag/v1.0.1) | GNU 交叉编译工具链           |
| **Conda / Miniconda** | Python 3.12                                                                | 构建工具隔离                |
| **Docker**            | Desktop 24+                                                                | Claude Code CLI 沙箱    |
| **Claude Code CLI**   | v2.1.146（在 Docker 中）                                                       | 被测大语言模型               |
| **API Key**           | Anthropic / 兼容                                                             | 例如 DeepSeek、Anthropic |

## 安装

### 1. 克隆本仓库

```bash
git clone https://github.com/NeonSpectre23/zephyr-workspace.git
cd zephyr-workspace
```

### 2. 拉取 Zephyr 源码和模块

Zephyr RTOS 源码及其模块不存储在本仓库中（体积大且版本固定）。使用 west 恢复它们：

**方案 A — 标准 west 流程：**

```bash
# 清理旧的 west 配置
rm -rf .west

# 使用 v4.4.0 官方 Zephyr manifest 初始化 west
west init -m https://github.com/zephyrproject-rtos/zephyr --mr v4.4.0

# 拉取所有项目（zephyr/、modules/、bootloader/、tools/）
west update
```

**方案 B — 浅克隆（推荐 GitHub 慢/不稳定时使用）：**

`west init` 会克隆 zephyr manifest 仓库的**全部 git 历史**（约 160 万对象）。在被墙/慢速网络（例如中国大陆）下通常会以 `fatal: early EOF` / `fetch-pack: unexpected disconnect` 失败。本基准测试从不使用 zephyr 的 git 历史——`inject_test.py` 只做 reset 和构建——所以浅克隆功能完全一样，下载量却小得多：

```bash
# 清理残留/半成品状态
rm -rf .west zephyr

# 只拉取 v4.4.0 快照（无历史）
git clone --depth 1 --branch v4.4.0 https://github.com/zephyrproject-rtos/zephyr zephyr

# 用本地仓库初始化 west（跳过全量历史克隆）
west init -l zephyr

# 拉取所有项目（zephyr/、modules/、bootloader/、tools/）
west update
```

> ⏱ 两种方式都会下载约 1 GB 源码。喝杯咖啡等吧。
> `west update` 可断点续传——如果某个项目以 `early EOF` 失败，重新执行即可。也可以通过 `git config --global http.postBuffer 524288000` 缓解大仓库克隆超时。

### 3. 配置 conda 环境

```bash
conda env create -f environment.yml
conda activate zephyr
```

或者使用激活脚本（自动处理 conda 激活 + 环境变量）：

```bash
source activate.sh
```

### 4. 安装 Zephyr SDK

```bash
wget https://github.com/zephyrproject-rtos/sdk-ng/releases/download/v1.0.1/zephyr-sdk-1.0.1_linux-x86_64_gnu.tar.xz
tar xf zephyr-sdk-1.0.1_linux-x86_64_gnu.tar.xz -C ~/
cd ~/zephyr-sdk-1.0.1/
./setup.sh
```

> 基准测试从 `ZEPHYR_SDK_INSTALL_DIR` 环境变量读取 SDK 路径，默认 `~/zephyr-sdk-1.0.1`。如果安装在其他位置：运行前执行 `export ZEPHYR_SDK_INSTALL_DIR=/path/to/your/sdk`。

环境就绪后的步骤——**构建 Docker 沙箱、配置 API Key、依赖自检、跑评测、白盒分析**——见 [`zephyr-claude/README.md`](zephyr-claude/README.md)。

## 数据集：712 个已验证任务

当前数据集为 **712 个通过两轮验证的 GOOD 任务**（oracle 注入通过测试 + 空实现注入让测试失败），由原始 884 个任务经负控制扫描筛除弱任务后得到。任务通过 `extract_zephyr_tests.py` 从 Zephyr RTOS 单元测试（`tests/unit/`、`tests/subsys/`、`tests/lib/`）中提取：

| 来源         | 优先级 | 说明      |
| ---------- | --- | ------- |
| `lib/`     | 最高  | 核心库函数   |
| `subsys/`  | 高   | 子系统 API |
| `arch/`    | 中   | 架构相关    |
| `kernel/`  | 中   | 内核 API  |
| `drivers/` | 低   | 设备驱动    |

每个任务包含：

- `task_id` — 唯一标识符
- `sut_function` — 需要实现的函数名
- `source_path` — 包含该函数的文件
- `masked_code` — 函数签名，函数体为 `/* TODO(agent) */`
- `oracle` — 参考实现路径
- `run_command` — 用于验证的 `west build` 命令
- `unit_test` — 用于判断测试通过/失败的 ZTEST 名称

## 结果与白盒分析

运行结果默认写 `zephyr-claude/results/results.jsonl`；多模型/多轮用 `ZEPHYR_CLAUDE_RUN_NAME=<run>` 时写 `results-<run>/results.jsonl` + `trajectory-<run>/`（JSONL，每行一条）：

```json
{
  "task_id": "1",
  "passed": true,
  "claude_completed": true,
  "runtime_s": 45.2,
  "illegal_changes": [],
  "error_category": "none",
  "patch": "...",
  "snapshot": { ... }
}
```

轨迹日志（Claude 会话 + diff + 验证输出）在对应 `trajectory[-<run>]/`。

**失败分类（白盒）与分析流水线**

- `zephyr-claude/whitebox.py`：失败分类成对齐 QSemOS/RIOT 的 L2 taxonomy + mechanism，输出 `whitebox_report.json`。
- `zephyr-claude/analyze_run.py`：复用流水线 `classify`（剔 illegal/假失败/实过并修正 L1）/ `casebook` / `prelabel` / `finalize` / `normalize`（重写 results 的 passed/error\_category，留 `.raw`）/ `audit`（口径自查）/ `selftest`；报告落 `results-<model>/whitebox_report.json`、自查落 `schema_audit.json`。
- **已产出**：`results-deepseek-v4-pro/`（712 全量，86 条真实失败）与 `results-glm/`（712 全量，27 条真实失败），各含 `annotation/`（casebook + annotations L1/L2/L3）。
- 省 token 抽样：`zephyr-bench/sample_tasks.py` 按 module L1 抽子集 → `runner.py --data <子集> --batch N`。

**通过率口径**：分母固定 **712**；`illegal_modifications`（指令遵循失败）与 harness 层失败（`timeout`/`watchdog`/`api_error`/`exception`，即 FF）**均计入分母、按失败计**（保守口径，两模型同口径）。而**机制/白盒统计剔除 FF 与 illegal**（与通过率不同口径，论文需分别标注）；白盒与 `illegal_changes` 须不相交，`analyze_run.py audit` 会断言此点。

### 测试结果（712 任务，同口径）

| 模型              | 通过  | 通过率   | 失败  | 指令遵循(ii) | harness FF | 真实失败   |
| --------------- | --- | ----- | --- | -------- | ---------- | ------ |
| deepseek-v4-pro | 612 | 86.0% | 100 | 8        | 6          | **86** |
| glm-5.1         | 669 | 94.0% | 43  | 10       | 6          | **27** |

> 通过率分母 = 712；`真实失败 = 失败 − 指令遵循 − FF`（即进白盒的条目）。详见 `results-<model>/whitebox_report.json`、`schema_audit.json`。

### 白盒分析结果（真实失败）

| 模型              | 真实失败 | test\_failure | crash | compile\_error | misunderstanding | hallucination |
| --------------- | ---- | ------------- | ----- | -------------- | ---------------- | ------------- |
| deepseek-v4-pro | 86   | 54            | 24    | 8              | 83               | 3             |
| glm-5.1         | 27   | 16            | 7     | 4              | 27               | 0             |

L2 分布（canonical，经 `L2_CONSOLIDATED` 合并）：

| L2                          | deepseek-v4-pro | glm-5.1 |
| --------------------------- | --------------- | ------- |
| `logic_deviation`           | 53              | 19      |
| `missing_conditional_guard` | 11              | 4       |
| `wrong_calculation`         | 9               | 2       |
| `extra_conditional_guard`   | 5               | 1       |
| `missing_error_path`        | 4               | 1       |
| `undefined_reference`（幻觉）   | 3               | 0       |
| `semantic_drift`            | 1               | 0       |
| **合计**                      | **86**          | **27**  |

> 观察：两模型失败都以 `logic_deviation` 为主；glm 通过率更高、且**无幻觉**（compile\_error 均为签名/类型类），deepseek 有 3 条编造符号的 `undefined_reference`。逐条 L1/L2/L3 见各 `results-<model>/annotation/annotations.jsonl`。

## 许可证

Zephyr RTOS 采用 [Apache 2.0](zephyr/LICENSE) 许可证。\
基准测试工具同样以该许可证条款提供。
