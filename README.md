# Zephyr RTOS 代码补全基准测试

用于评估大语言模型（Claude / DeepSeek 等）在 **Zephyr RTOS C 函数补全**任务上的表现——模型看到一份函数体被替换为 `/* MASKED */` 的 Zephyr 源文件，需要实现该函数，最终通过 `west build` 编译 + 运行真实单元测试来验证结果。

> 从 RIOT-OS 基准测试模式移植而来，适配 Zephyr 4.4.0，已通过 Anthropic 兼容 API 配合 DeepSeek v4 Flash 测试。
>
> **说明：** 本仓库只包含基准测试框架和数据集。Zephyr RTOS 源码及其模块**不包含在内**——它们会在安装时由 `west init` + `west update` 自动拉取。

## 仓库内容（git 内 vs. west 拉取）

| 包含在 git 中 | 由 `west update` 拉取 |
|---|---|
| `zephyr-claude/` — 基准测试框架 | `zephyr/` — Zephyr RTOS 源码 |
| `zephyr-bench/` — 884 个任务 + oracle | `modules/` — HAL、加密、文件系统 |
| `activate.sh`、`setup.sh`、`environment.yml` | `bootloader/` — MCUboot |
| `claude_settings.template`、`.gitignore` | `tools/edtt/` — 蓝牙测试工具 |
| `README.md`、`.github/workflows/ci.yml` | |

### 目录结构

```
zephyr-workspace/
├── zephyr/               ← Zephyr RTOS v4.4.0（west 拉取，不在 git 中）
├── zephyr-bench/         ← 基准数据集（884 个任务）+ 构建脚本  ★ 在 git 中
│   ├── zephyr_tasks.c.jsonl    627 个任务（C 文件）
│   ├── zephyr_tasks.h.jsonl    257 个任务（头文件）
│   ├── oracles/                884 个已验证的参考实现
│   ├── build_zephyr_dataset.py 从单元测试构建数据集
│   ├── extract_zephyr_tests.py ZTEST 块提取
│   └── verify_zephyr_tests.py  Oracle 注入与验证
├── zephyr-claude/        ← 评估框架  ★ 在 git 中
│   ├── agent.py               主流水线：mask → Claude → verify
│   ├── mask.py                tree-sitter AST 操作
│   ├── runner.py              CLI 入口
│   ├── prompt.py              LLM 提示词模板
│   ├── config.py              全局配置
│   ├── Dockerfile             沙箱镜像（Ubuntu + west + Claude Code CLI）
│   ├── analyze_results.py     结果分析与统计
│   ├── final_analysis.py      失败模式深入分析
│   └── settings.json          benchmark 专用 API 配置（gitignore，见 §6）
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

| 依赖 | 版本 / 位置 | 说明 |
|---|---|---|
| **Zephyr SDK** | [v1.0.1](https://github.com/zephyrproject-rtos/sdk-ng/releases/tag/v1.0.1) | GNU 交叉编译工具链 |
| **Conda / Miniconda** | Python 3.12 | 构建工具隔离 |
| **Docker** | Desktop 24+ | Claude Code CLI 沙箱 |
| **Claude Code CLI** | v2.1.146（在 Docker 中） | 被测大语言模型 |
| **API Key** | Anthropic / 兼容 | 例如 DeepSeek、Anthropic |

## 安装

### 1. 克隆本仓库

```bash
git clone https://github.com/YOUR_USER/zephyr-workspace.git
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
wget https://github.com/zephyrproject-rtos/sdk-ng/releases/download/v1.0.1/zephyr-sdk-1.0.1_linux-x86_64.tar.xz
tar xf zephyr-sdk-1.0.1_linux-x86_64.tar.xz -C ~/
cd ~/zephyr-sdk-1.0.1/
./setup.sh
```

> 基准测试从 `ZEPHYR_SDK_INSTALL_DIR` 环境变量读取 SDK 路径，默认 `~/zephyr-sdk-1.0.1`。如果安装在其他位置：运行前执行 `export ZEPHYR_SDK_INSTALL_DIR=/path/to/your/sdk`。

### 5. 构建 Docker 沙箱

```bash
cd zephyr-claude
docker build -t zephyr-sandbox:latest .
```

> 沙箱容器以 `--network host` 启动（`ZEPHYR_CLAUDE_NETWORK`，默认 `host`）——这是必需的，因为容器内的 Claude Code 要调用 LLM API（如 `api.deepseek.com`）。如需临时改用其他网络（如 `bridge`）：`export ZEPHYR_CLAUDE_NETWORK=bridge`。同时启动命令带 `--disallowedTools "WebSearch,WebFetch"`，禁用 Claude 的 web 搜索/抓取工具，防止被测模型联网查答案。注意这只禁用了 web 工具，容器内 `Bash` 未禁用。

### 6. 配置 API Key

基准测试在 Docker 容器里运行 Claude Code CLI，需要配置 LLM 的 API 地址和 Key。配置文件按以下优先级查找（见 `zephyr-claude/config.py` 的 `_default_claude_settings()`），也可用环境变量 `ZEPHYR_CLAUDE_SETTINGS` 强制指定任意配置文件：

1. **`zephyr-claude/settings.json`** — benchmark 专用配置，**推荐使用**。它不在任何 `.claude/` 目录下，交互式 Claude 根本不会读取，因此改基准测试的模型/Key 完全不影响你日常使用（例如 cc-switch 的配置）；
2. **`~/.claude/settings.json`** — 你的交互式 Claude 配置，作为回退项。

配置方法（benchmark 专用）：

```bash
cd zephyr-workspace
cp claude_settings.template zephyr-claude/settings.json
# 编辑 zephyr-claude/settings.json → 把 "sk-xxx" 替换成你的真实 Key
```

该文件已被 `.gitignore` 排除（`**/settings.json`），不会提交到仓库。

支持的 API 提供方（通过 `ANTHROPIC_BASE_URL` 配置）：
- **DeepSeek**：`https://api.deepseek.com/anthropic`
- **Anthropic**：`https://api.anthropic.com`（留空或不设置）
- **任意 Anthropic 兼容代理**

默认模型为 `deepseek-v4-flash`——按需修改 `ANTHROPIC_MODEL`（连同 `ANTHROPIC_DEFAULT_*_MODEL` 等一起改，运行时会全部注入容器）。

> 每次跑任务时 `agent.py` 会把选中的配置文件复制进容器内 Claude 的 `claude_home/settings.json`，并追加一个 `claude-on-completion` 钩子用于检测任务完成，同时把其中 `ANTHROPIC_*` 环境变量通过 `-e` 传入容器。改模型/Key 只需改配置文件，不用改代码。

### 7. 验证依赖

```bash
cd zephyr-claude
python runner.py --check
```

预期输出：
```
=== Zephyr-Claude Dependency Check ===
  [OK] Docker daemon
  [OK] Docker image 'zephyr-sandbox:latest'
  [OK] tree-sitter (pip package)
  [OK] Claude settings
  [OK] Zephyr repo structure
  [OK] Zephyr SDK
  [OK] Dataset 'zephyr_tasks.c.jsonl'
  [OK] Dataset 'zephyr_tasks.h.jsonl'

All checks passed.
```

## 使用

### 运行单个任务

```bash
cd zephyr-claude
python runner.py --task-id 1
```

### 运行批量任务

```bash
python runner.py --batch 10
```

### 断点续跑（跳过已通过的任务）

```bash
python runner.py --batch 100 --resume
```

### 从指定任务开始

```bash
python runner.py --batch 50 --start 200
```

### 验证数据集（注入 oracle → 构建 → 测试）

```bash
python inject_test.py --batch 5
```

### 分析结果

```bash
python analyze_results.py
```

## 数据集：884 个任务

任务通过 `extract_zephyr_tests.py` 从 Zephyr RTOS 单元测试（`tests/unit/`、`tests/subsys/`、`tests/lib/`）中提取：

| 来源 | 优先级 | 说明 |
|---|---|---|
| `lib/` | 最高 | 核心库函数 |
| `subsys/` | 高 | 子系统 API |
| `arch/` | 中 | 架构相关 |
| `kernel/` | 中 | 内核 API |
| `drivers/` | 低 | 设备驱动 |

每个任务包含：
- `task_id` — 唯一标识符
- `sut_function` — 需要实现的函数名
- `source_path` — 包含该函数的文件
- `masked_code` — 函数签名，函数体为 `/* TODO(agent) */`
- `oracle` — 参考实现路径
- `run_command` — 用于验证的 `west build` 命令
- `unit_test` — 用于判断测试通过/失败的 ZTEST 名称

## 错误分类

| 分类 | 含义 | 常见原因 |
|---|---|---|
| `compile_error` | 构建失败 | API 名称错误、预处理不匹配、语法错误 |
| `test_failure` | 能编译但测试失败 | 算法边界情况、常量错误 |
| `crash` | 运行时崩溃 | 空指针、未初始化字段 |
| `illegal_modifications` | 修改了目标函数之外的代码 | `#undef`、全局修改 |
| `timeout` | `west build` 超过 5 分钟 | 依赖树过大 |
| `watchdog` | Claude 会话超时（8 小时） | 卡死 / 死循环 |

## 结果

结果存储在 `zephyr-claude/results/results.jsonl`（JSONL 格式，每行一条结果）：

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

轨迹日志（完整的 Claude 会话 + diff + 验证输出）存储在 `zephyr-claude/trajectory/`。

## 许可证

Zephyr RTOS 采用 [Apache 2.0](zephyr/LICENSE) 许可证。  
基准测试工具同样以该许可证条款提供。
