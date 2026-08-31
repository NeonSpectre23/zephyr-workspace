# zephyr-bench — Zephyr 代码补全基准数据集

本目录是 **Zephyr RTOS 代码补全基准**的数据集模块：从 Zephyr 单元测试出发提取被测函数（SUT），生成"遮蔽"后的补全任务与参考实现（oracle），并经**两轮验证**（答案注入 + 负控制）筛出真正可用的任务。

**当前最终数据集：712 个验证通过的 GOOD 任务**（每个任务都满足：oracle 注入 → 测试通过；空实现注入 → 测试失败）。

> 评估框架见 [`../zephyr-claude/README.md`](../zephyr-claude/README.md)。

## 目录内容

| 文件/目录 | 说明 |
|---|---|
| `zephyr_tasks.c.jsonl` / `.h.jsonl` | **最终数据集**（712 任务：493 `.c` + 219 `.h`） |
| `oracles/` | 最终参考实现（712 个，与 task_id 一一对应） |
| `zephyr_tests.jsonl` | 从单元测试提取的测试元数据（中间产物） |
| `build_zephyr_dataset.py` | 任务构建脚本（tree-sitter 提取 SUT + 遮蔽 + 生成 oracle） |
| `extract_zephyr_tests.py` | ZTEST/ZTEST_F 块提取脚本 |
| `verify_zephyr_tests.py` | 数据集验证脚本（west build / GCC 轻量两种模式） |
| `filter_dataset.py` | **按验证判定筛出最终数据集**（只保留 GOOD，重编号，重建 oracles） |
| `mask_engine.py` | 函数体遮蔽工具（`mask_function`） |


## 数据流水线

```
extract_zephyr_tests.py       build_zephyr_dataset.py       verify/filter
  扫描 Zephyr 单元测试    →    生成任务 + oracle        →     验证 → 只留 GOOD
  (ZTEST 块提取)               (tree-sitter + 遮蔽)          (最终 712)
        │                            │                            │
        ▼                            ▼                            ▼
  zephyr_tests.jsonl         zephyr_tasks.c/.h.jsonl      filter_dataset.py
                             + oracles/NNN.{c,h}          按 neg_control 判定筛除
```

- **构建**（`build_zephyr_dataset.py`）:读测试元数据 → 找出测试调用的函数（SUT）→ 按目录优先级在源码里定位定义 → 遮蔽函数体 → 写 `zephyr_tasks.*.jsonl` + `oracles/`
- **验证**（`zephyr-claude/neg_control.py` + `inject_test.py`）:两轮验证每个任务（详见 zephyr-claude README）
- **筛选**（`filter_dataset.py`）:按验证判定只保留 GOOD，重编号、重建 oracles，产出最终数据集

## 使用方式

> 需先有 Zephyr 源码（`../zephyr`）并激活 conda 环境。

### 1. 提取测试元数据

```bash
python extract_zephyr_tests.py --zephyr-root ../zephyr \
    --output zephyr_tests.jsonl --categories unit,lib,subsys
```

### 2. 构建数据集

```bash
python build_zephyr_dataset.py --repo ../zephyr \
    --tests zephyr_tests.jsonl --out-prefix zephyr_tasks
```

### 3. 验证数据集

```bash
# west build 模式（真实构建，慢但准）
python verify_zephyr_tests.py --input zephyr_tasks.c.jsonl \
    --zephyr-root ../zephyr --out-dir .

# 轻量模式（GCC 编译函数体，快，适合 CI）
python verify_zephyr_tests.py --input zephyr_tasks.c.jsonl \
    --zephyr-root ../zephyr --out-dir . --lightweight
```

### 4. 筛出最终数据集（验证后）

```bash
# 输入 neg_control 判定结果，输出仅 GOOD 的最终数据集
python filter_dataset.py \
    --orig ../zephyr-claude/final_verdicts.json \
    --bench .
```

> 完整验证（两轮判定）在 `zephyr-claude/` 里做（`neg_control.py`），`filter_dataset.py` 消费其结果。原数据集会自动备份为 `*-full.jsonl` / `oracles-full/`。

## 任务数据格式

每个任务一行 JSON：

| 字段 | 说明 |
|---|---|
| `task_id` | 唯一标识（1..712） |
| `source_path` | 被测函数所在的 Zephyr 源文件 |
| `sut_function` | 需要实现的函数名 |
| `oracle` | 参考实现路径（相对 `zephyr-bench/`） |
| `masked_code` | 遮蔽后的代码：函数签名 + `{`，函数体为 `/* TODO(agent) */` |
| `unit_test` | 用于判定通过/失败的 ZTEST 名称 |
| `run_command` | 验证命令（`west build` + 运行测试） |
| `suite` / `zephyr_test_module` | 测试套件 / 模块名 |

示例（task 1）：

```json
{
  "task_id": "1",
  "source_path": "lib/utils/hex.c",
  "sut_function": "bin2hex",
  "oracle": "oracles/1.c",
  "masked_code": "size_t bin2hex(const uint8_t *buf, size_t buflen, char *hex, size_t hexlen)\n{\n    /* TODO(agent) */\n}",
  "unit_test": "test_bin2hex",
  "run_command": "cd tests/unit/hex && west build -b unit_testing -p -d build -- -DZEPHYR_TOOLCHAIN_VARIANT=host -DM64_MODE=1 && ./build/testbinary",
  "suite": "hex",
  "zephyr_test_module": "hex"
}
```

## 遮蔽逻辑（mask_engine.py）

`mask_function(func_text)` 保留函数签名与 `{` / `}`，把函数体替换为 `/* TODO(agent) */`；找不到配对 `{}` 时返回 `None`（该函数不收录）。

## 任务来源与优先级

SUT 由测试文件**实际调用**的函数推导（tree-sitter 解析 `call_expression`），按目录优先级挑选定义：

| 目录 | 优先级 |
|---|---|
| `lib/` | 最高 |
| `subsys/` | 高 |
| `kernel/`、`arch/` | 中 |
| `drivers/` | 低 |

排除 `tests` / `build` / `doc` / `scripts` / `boards` / `soc` 等目录与框架函数（`zassert_*`、`printk`、`memcpy` 等）。

## 最终数据集现状（712 任务）

| 子系统 | 数量 |
|---|---|
| `subsys` | 345 |
| `include`（头文件内联） | 214 |
| `lib` | 137 |
| `kernel` | 13 |
| `modules` / `samples` / `drivers` | 各 1 |

> ⚠️ `drivers` 覆盖极少——驱动类任务的测试大多不真验证函数，在验证阶段被 WEAK 筛除。数据集偏向 `subsys`/`lib` 核心库，跨模块覆盖需在论文中如实披露。

## 依赖

- Python 3.12（conda 环境 `zephyr`，见顶层 `environment.yml`）
- `tree-sitter` + `tree-sitter-c`（`build_zephyr_dataset.py` / `mask_engine.py` 需要）
- Zephyr 源码（`../zephyr`，west 拉取）
- Zephyr SDK（`verify_zephyr_tests.py` west build 模式需要）
