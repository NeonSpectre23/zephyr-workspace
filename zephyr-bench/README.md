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
| `sample_tasks.py` | **按 module L1 分层抽样**，产出可给 `runner.py --data` 用的子集 jsonl |


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
# 输入 = neg_control 的判定结果(JSON: task_id -> verdict)，输出仅 GOOD 的最终数据集
python filter_dataset.py \
    --orig <neg_control 结果.json> \
    [--recheck <重跑判定.json>] \
    --bench .
```

> 完整验证（两轮判定）在 `zephyr-claude/` 里做（`neg_control.py` 产出 `neg_control_results.json` 等），`filter_dataset.py` 消费其结果；判定合并规则：`--recheck` 中的任务以其为准，其余用 `--orig`。
> 筛选时默认会把原数据集备份为 `*-full.jsonl` / `oracles-full/`（当前仓库未保留该备份，仅描述行为）。

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

## 最终数据集现状（712 任务，module L1 划分）

module L1 与 `zephyr-claude/whitebox.py` 的 `classify_module` **同口径**（顶层目录 + `include/zephyr/<sub>/` 头文件内联归并到子模块）——`whitebox_report.json` 的 `module` 字段与下面的分层抽样均用这套 L1：

| module | 数量 | 占比 | .c / .h | 说明 / 代表函数 |
|---|---|---|---|---|
| `subsys` | 345 | 48.5% | 341 / 4 | crc、fs/fcb、pm、usb、modem、mem_blocks 等 |
| `lib` | 137 | 19.2% | 136 / 1 | hex、cbprintf、heap、smf、onoff、bitarray、rb 等 |
| `sys` | 90 | 12.6% | 0 / 90 | `include/zephyr/sys/*.h`：atomic、spsc_pbuf、ring_buffer、linear_range |
| `net` | 66 | 9.3% | 0 / 66 | 几乎全为 `include/zephyr/net_buf.h`(62) + 少量 `net/*.h` |
| `kernel` | 23 | 3.2% | 13 / 10 | device/驱动模型 + 内核 API（`device.h`/`kernel.h` 归 kernel）|
| `rtio` | 15 | 2.1% | 0 / 15 | `include/zephyr/rtio/*.h` |
| `zbus` | 11 | 1.5% | 0 / 11 | `include/zephyr/zbus/*.h` |
| `logging` | 10 | 1.4% | 0 / 10 | `include/zephyr/logging/*.h` |
| `drivers` | 4 | 0.6% | 1 / 3 | uart_emul、uhc |
| `fs` / `modem` / `shell` | 各 2 | 0.3% | 全 .h | fs.h、modem_chat/ubx、shell_backend |
| `modules` / `samples` | 各 1 | 0.1% | .c | lvgl_init、usbd sample |
| `storage` / `math` / `random` | 各 1 | 0.1% | .h | flash_map、interpolation、sys_rand32 |

> ⚠️ 口径说明：表中将头文件内联按子系统归并（`include/zephyr/<sub>/` → `<sub>`），故出现 `sys`/`net`/`rtio`… 这些子系统级 L1；若按"`include` 单独一组"的粗口径则是 214 个。`net` 的 66 条几乎全是 `net_buf.h`（62），并非网络栈整体，跨模块解读时注意。`drivers` 覆盖极少——驱动类任务多因"测试不真验证函数"在验证阶段被 WEAK 筛除；数据集整体偏向 `subsys`/`lib` 核心库，模块覆盖不均需在论文中如实披露。

### 模块维度与既有 track 的对齐声明（整改清单 §四要求）

跨 track 模块图**只做 per-model 呈现、不跨 track 相加**；对齐性显式声明如下（不可对齐项须在论文中如实披露）：

| Zephyr module L1 | 与 QSem / RIOT | 说明 |
|---|---|---|
| `kernel` | 概念对齐 QSem `kernel` | — |
| `subsys` | 概念对齐 RIOT `sys/*`（粒度更粗）| 子系统不再细分 |
| `drivers` | 概念对齐（RIOT 驱动分散于多个模块）| — |
| `lib` / `arch` / `modules` / `samples` | **Zephyr 独有，不可对齐** | 无对应项 |
| `<subsystem>`（`sys`/`net`/`rtio`/`zbus`/`logging`…）| **Zephyr 特有，不可对齐** | 来自 `include/zephyr/<sub>/` 头文件内联的归并 |

### 按模块分层抽样（省 token / 保证覆盖）

多模型（如 GLM）全量跑太耗 token 时，可按 module L1 分层抽子集给 runner：

```bash
cd zephyr-bench
python sample_tasks.py --total 100 --strategy proportional_min --seed 0 \
    --out zephyr_tasks.sample.jsonl      # 每非空模块至少 1 条,其余按占比
# 可选 --module subsys lib sys net kernel / --print-ids / 其它 --seed
cd ../zephyr-claude
ZEPHYR_CLAUDE_RUN_NAME=glm-sample \
  python runner.py --data ../zephyr-bench/zephyr_tasks.sample.jsonl --batch 100
```

## 依赖

- Python 3.12（conda 环境 `zephyr`，见顶层 `environment.yml`）
- `tree-sitter` + `tree-sitter-c`（`build_zephyr_dataset.py` / `mask_engine.py` 需要）
- Zephyr 源码（`../zephyr`，west 拉取）
- Zephyr SDK（`verify_zephyr_tests.py` west build 模式需要）
- `sample_tasks.py` 复用 `../zephyr-claude/whitebox.py` 的 `classify_module`（module L1），无需 Zephyr SDK
