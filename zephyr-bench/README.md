# zephyr-bench — Zephyr 代码补全基准数据集

本目录是 **Zephyr RTOS 代码补全基准**的数据集构建与验证模块：从 Zephyr 源码的单元测试出发，提取被测函数（SUT），生成「遮蔽（masked）」后的补全任务，并为每个任务提供已验证的参考实现（oracle）。数据由上层评估框架 `zephyr-claude/` 消费。

> 与顶层仓库的关系见 [`../README.md`](../README.md)。

## 数据流水线

```
┌─────────────────────────┐     ┌──────────────────────────┐     ┌────────────────────────┐
│ extract_zephyr_tests.py │     │  build_zephyr_dataset.py │     │ verify_zephyr_tests.py │
│ 扫描 Zephyr 单元测试      │────►│  生成补全任务 + oracle     │────►│  验证数据集可编译/可测试  │
│ ZTEST/ZTEST_F 块提取     │     │  (tree-sitter + 遮蔽)     │     │  west build / GCC 轻量   │
└─────────────────────────┘     └──────────────────────────┘     └────────────────────────┘
            │                              │                              │
            ▼                              ▼                              ▼
   zephyr_tests.jsonl          zephyr_tasks.c/.h.jsonl           verify_results.json
   （测试元数据）                    + oracles/NNN.{c,h}               + verify_logs/
```

| 步骤 | 脚本 | 输入 | 输出 |
|---|---|---|---|
| 1. 提取测试 | `extract_zephyr_tests.py` | Zephyr 源码 `tests/unit`、`tests/lib`、`tests/subsys` | `zephyr_tests.jsonl`（测试元数据） |
| 2. 构建任务 | `build_zephyr_dataset.py` | `zephyr_tests.jsonl` + Zephyr 源码 | `zephyr_tasks.c.jsonl`、`zephyr_tasks.h.jsonl`、`oracles/` |
| 3. 验证数据集 | `verify_zephyr_tests.py` | 任务文件 + Zephyr 源码 | `verify_results.json`、`verify_logs/` |

## 目录内容

| 文件/目录 | 说明 |
|---|---|
| `zephyr_tasks.c.jsonl` | **C 文件任务**（627 个），每行一个 JSON 任务 |
| `zephyr_tasks.h.jsonl` | **头文件任务**（257 个），每行一个 JSON 任务 |
| `zephyr_tests.jsonl` | 中间产物：从单元测试提取的测试元数据 |
| `oracles/` | 884 个参考实现（`N.c` / `N.h`），文件名编号与 `task_id` 对应 |
| `build_zephyr_dataset.py` | 任务构建脚本（依赖 tree-sitter） |
| `extract_zephyr_tests.py` | ZTEST/ZTEST_F 块提取脚本 |
| `verify_zephyr_tests.py` | 数据集验证脚本（两种模式） |
| `mask_engine.py` | 函数体遮蔽工具（`mask_function`），供构建脚本调用 |

## 使用方式

> 均需先准备好 Zephyr 源码（`../zephyr`），并激活 conda 环境（见顶层 `environment.yml`）。

### 1. 提取测试元数据

```bash
cd zephyr-bench
python extract_zephyr_tests.py --zephyr-root ../zephyr \
    --output zephyr_tests.jsonl \
    --categories unit,lib,subsys
```

### 2. 构建数据集

```bash
python build_zephyr_dataset.py \
    --repo ../zephyr \
    --tests zephyr_tests.jsonl \
    --out-prefix zephyr_tasks
```

生成 `zephyr_tasks.c.jsonl`、`zephyr_tasks.h.jsonl`，并把参考实现写入 `oracles/`。

### 3. 验证数据集

两种模式：

```bash
# west build 模式（真实构建 + 运行 ztest，慢但准确）
python verify_zephyr_tests.py --input zephyr_tasks.c.jsonl \
    --zephyr-root ../zephyr --out-dir . --timeout 600

# 轻量模式（GCC 编译函数体 + mock 的 ztest 头文件，快，适合 CI）
python verify_zephyr_tests.py --input zephyr_tasks.c.jsonl \
    --zephyr-root ../zephyr --out-dir . --lightweight
```

结果输出到 `verify_results.json`（每个测试一个状态）与 `verify_logs/`（构建日志）。

## 任务数据格式（zephyr_tasks.*.jsonl）

每个任务一行 JSON，字段如下：

| 字段 | 说明 |
|---|---|
| `task_id` | 唯一标识（字符串，1..884） |
| `source_path` | 被测函数所在的 Zephyr 源文件（相对仓库根） |
| `sut_function` | 需要实现的函数名 |
| `oracle` | 参考实现路径（相对 `zephyr-bench/`） |
| `masked_code` | 遮蔽后的代码：保留函数签名 + `{`，函数体替换为 `/* TODO(agent) */` |
| `unit_test` | 用于判定通过/失败的 ZTEST 名称 |
| `run_command` | 验证命令（`west build` + 运行测试二进制） |
| `suite` | 测试套件名 |
| `zephyr_test_module` | 测试模块名 |

示例：

```json
{
  "task_id": "1",
  "source_path": "kernel/atomic_c.c",
  "sut_function": "atomic_get",
  "oracle": "oracles/1.c",
  "masked_code": "atomic_val_t atomic_get(const atomic_t *target)\n{\n    /* TODO(agent) */\n}",
  "unit_test": "test_int_wrap_around",
  "run_command": "cd tests/lib/lockfree && west build -b native_sim/native/64 -p -d build -- -DZEPHYR_TOOLCHAIN_VARIANT=host && ./build/zephyr/zephyr.exe",
  "suite": "spsc",
  "zephyr_test_module": "lockfree"
}
```

## 遮蔽逻辑（mask_engine.py）

`mask_function(func_text)` 把函数体替换为占位符：

- **保留**：函数签名、开括号 `{`、闭括号 `}` 所在行；
- **替换**：两者之间的函数体为一行 `/* TODO(agent) */`；
- 若找不到配对的 `{}` 则返回 `None`（该函数不会被收录为任务）。

## 任务来源与优先级

SUT 函数由测试文件中**被调用的函数**推导而来（tree-sitter 解析 `call_expression`），再在 Zephyr 源码中按目录优先级挑选定义：

| 目录 | 优先级 |
|---|---|
| `lib/` | 最高（核心库函数） |
| `subsys/` | 高 |
| `kernel/`、`arch/` | 中 |
| `drivers/` | 低 |

被排除的目录：`tests`、`build`、`doc`、`scripts`、`boards`、`soc`、`dts`、`cmake` 等（这些是测试或构建基础设施，不是 SUT 来源）。框架/标准库函数（`zassert_*`、`printk`、`memcpy` 等）也会被过滤。

## 统计

| 项目 | 数量 |
|---|---|
| C 文件任务 | 627 |
| 头文件任务 | 257 |
| **任务总数** | **884** |
| oracle 文件 | 884 |
| 测试元数据条目（zephyr_tests.jsonl） | 1824 |

## 依赖

- Python 3.12（conda 环境 `zephyr`，见顶层 `environment.yml`）
- `tree-sitter` + `tree-sitter-c`（仅 `build_zephyr_dataset.py` 需要）
- Zephyr 源码（`../zephyr`，由 `west update` 拉取）
- Zephyr SDK（`verify_zephyr_tests.py` west build 模式需要）
