# zephyr-claude — Zephyr 代码补全评估框架

本目录是 **Zephyr RTOS 代码补全基准**的评估框架：读取 `zephyr-bench/` 生成的任务数据集，把目标函数体遮蔽为 `/* MASKED */`，在 Docker 沙箱里调用 Claude Code CLI 让被测模型补全，再把模型输出注入真实 Zephyr 源码，通过 `west build` + 运行单元测试来判定通过/失败。

> 适配自 riot-claude。数据集的构建与验证见 [`../zephyr-bench/README.md`](../zephyr-bench/README.md)。

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
| `inject_test.py` | **答案注入验证**：注入 oracle → west build → 测试,验证数据集"能过" |
| `neg_control.py` | **负控制验证**：再注入空实现,找出测试未真正验证函数的"弱任务" |
| `analyze_results.py` | 结果统计(通过率/错误分类/运行时/按目录/按文件类型) |
| `final_analysis.py` | 失败 case 深入分析(读 trajectory 逐例复盘) |
| `Dockerfile` | 沙箱镜像:Ubuntu 24.04 + west + Claude Code CLI |
| `settings.json` | benchmark 专用 API 配置(Key/模型/Base URL,**已被 gitignore**) |

运行产物: `results/results.jsonl`(每次任务一行结果)、`trajectory/task_{id}.log`(完整会话轨迹)、`neg_control_results.json` / `neg_control.log`。

## 工作原理(agent.py 流水线)

```
① hard_reset_repo()            git reset --hard + clean,恢复 Zephyr 源码
② _prepare_workspace()         只复制目标源文件到工作区,apply_mask 把函数体换成 /* MASKED */
③ _capture_snapshot()          记录 git HEAD、镜像摘要、模型名、prompt 哈希、数据集哈希
④ _run_claude()                Docker 容器运行 Claude Code CLI 补全目标函数
⑤ 读 target.c + diff           masked 版本 vs 模型输出
⑥ 完整性检查                   AST 归一化等价性(只许改目标函数体)+ 函数体禁用关键字检查
⑦ _verify_tests()              把模型代码注入真实源码,west build + 跑 ztest,解析 PASS/FAIL
⑧ 记录结果 + 清理              写入 results.jsonl / trajectory/,git reset 还原
```

### 沙箱隔离(agent.py)

- 容器参数:`--cap-drop ALL`、`--security-opt no-new-privileges`、`--pids-limit`、`--memory`/`--cpus` 限额
- 整个 `zephyr/` 源码树**只读挂载** `/workspace:ro`,仅目标文件以 `rw` 挂载到原路径
- `--disallowedTools "WebSearch,WebFetch"` 禁用模型联网查答案
- 网络模式默认 `host`(config.py 的 `CLAUDE_NETWORK_MODE`,可用环境变量 `ZEPHYR_CLAUDE_NETWORK` 覆盖)

### 完整性/防作弊检查(mask.py)

- **AST 归一化等价性** `check_ast_integrity`:把 masked 和 final 两个版本都做归一化(函数体→占位符、参数名→泛型、签名空白归一),比较是否一致——发现目标函数之外被修改
- **函数体关键字检查** `check_body_integrity`:禁止函数体内出现 `#define`、`#include`、`typedef`、`__asm__` 等
- 违规 → `error_category = illegal_modifications`

### 结果判定(agent.py `_verify_tests`)

通过 = 无编译错误 + 无崩溃 + 目标 `unit_test` 无 FAIL + `PROJECT EXECUTION SUCCESSFUL`。

错误分类:

| error_category | 含义 |
|---|---|
| `none` | 通过 |
| `illegal_modifications` | 修改了目标函数之外 / 体内含禁用关键字 |
| `compile_error` | west build 失败 |
| `crash` | 运行崩溃(segfault/Aborted/panic) |
| `test_failure` | 编译过但测试失败 |
| `test_not_executed` | 测试没有真正执行 |
| `timeout` / `watchdog` | build 超时 / Claude 会话超时 |
| `api_error` | Claude 未完成(API 异常) |

## 使用

### 依赖自检

```bash
cd zephyr-claude
python runner.py --check
```

### 运行单任务 / 批量

```bash
python runner.py --task-id 1
python runner.py --batch 10
```

### 断点续跑(跳过已通过任务)

```bash
python runner.py --batch 100 --resume
```

### 从指定任务开始

```bash
python runner.py --batch 50 --start 200
```

### 数据集质量验证(答案注入 + 负控制)

```bash
# ① 答案注入 inject_test.py:注入 oracle(正确答案)应能通过测试
python inject_test.py --batch 20

# ② 负控制 neg_control.py:注入空实现应让测试失败(找出"假验证"弱任务)
python neg_control.py --batch 20
```

两个脚本都支持 `--resume` 断点续跑:`inject_test.py` 自动写 `inject_test.log`,`neg_control.py` 写 `neg_control.log`。详见各自的 `--help`。

**只重跑特定判定的任务**(如修 bug 后重跑 NEG_COMPILE_FAIL):

```bash
# 从结果文件(neg_control_results.json)选出 NEG_COMPILE_FAIL 的任务重跑
python neg_control.py --recheck NEG_COMPILE_FAIL \
    --results neg_control_results.json \
    --out neg_control_recheck.json \
    --log neg_control_recheck.log
```

**产出最终数据集**(验证后只保留 GOOD,重编号、重建 oracles):

```bash
# zephyr-bench/filter_dataset.py:输入判定结果,输出仅 GOOD 的最终数据集
python zephyr-bench/filter_dataset.py \
    --orig zephyr-claude/final_verdicts.json \
    --bench zephyr-bench
```

> 完整验证流水线:`neg_control` 扫描 → 按判定筛除 WEAK/ORACLE_FAIL/NCF → `filter_dataset.py` 只留 GOOD → 重编号。原数据集自动备份为 `*-full.jsonl` / `oracles-full/`。

### neg_control.py 负控制工作流程

**原理**：一个测试如果"真正验证"了目标函数,那么注入正确答案(oracle)时它必须通过,注入空实现(stub)时它必须失败。若两者都能通过,说明该测试没在验证这个函数——这类任务在任何模型评测里都"恒通过",会虚增通过率。

**逐任务流程**：

```
git_reset() 恢复 zephyr 源码
  ↓
inject_oracle(task)        注入 oracles/NNN.c 的正确答案
  ├─ 找不到函数            → ORACLE_SKIP
  └─ run_test() 测试通过吗?
       ├─ 不过              → ORACLE_FAIL   ← 数据集缺陷(run_command/unit_test/oracle 错)
       └─ 通过 ↓
          git_reset() 再次恢复
          inject_stub(task)  注入"能编译的空实现"
          └─ run_test() 空实现能过吗?
               ├─ 能过        → WEAK  ★测试没验证函数(假验证)
               ├─ 编译失败    → NEG_COMPILE_FAIL  ← 多半是 stub 生成/链接问题,需看日志
               └─ 不能过      → GOOD  测试真的验证了函数
```

**判定表**：

| 判定 | 含义 | 处置建议 |
|---|---|---|
| `GOOD` | oracle 过、空实现不过 → 测试真验证函数 | ✅ 保留 |
| `WEAK` | oracle 过、空实现也过 → 测试没验证函数 | ❌ 修或弃 |
| `ORACLE_FAIL` | oracle 注入后测试不过 → 数据集缺陷 | ❌ 修或弃 |
| `ORACLE_SKIP` | 函数定义找不到 | ❌ 修 source_path/sut_function |
| `NEG_COMPILE_FAIL` | 空实现没编译 → 工具限制(常见 `undefined reference` 链接错误) | ⚙️ 修 stub,任务本身可能有效 |

**空实现(stub)生成策略**(`neg_control.py` 的 `_stub_return`):按返回类型返回一个"显然不是正确实现"的值,让真验证函数的测试失败

| 返回类型 | stub return |
|---|---|
| `void` | `return;` |
| 指针(`T *`) | `return NULL;` |
| 无符号/尺寸(`size_t`/`uint*_t`) | `return 0;` |
| 浮点(`float`/`double`) | `return 0.0;` |
| 有符号整型(`int`/`long`/`ssize_t`) | `return -1;`(让 `err == 0` 的成功断言失败) |

> 早期版本一律 `return (类型){0};`,对"返回 0 = 成功"的 API(如 `bt_enable`、`bt_le_scan_start`)空实现恰好通过成功断言 → 误判 WEAK;同时指针返回类型提取漏掉 `*`(如 `struct log_backend *` 被当成 `struct log_backend`)。现已修复:指针正确补星号、有符号返回 -1、未知 typedef 返回 0(避免 `-Wsign-conversion` 警告)。

**诊断日志**:每轮 build 的完整 stdout/stderr 保存在 `neg_control_logs/task_{id}_oracle.log` 和 `task_{id}_stub.log`,`NEG_COMPILE_FAIL` / `ORACLE_FAIL` 可直接看日志定位。

**验证历程与最终结果**(2026-08):

| 阶段 | 任务数 | 说明 |
|---|---|---|
| 生成 | 884 | 原始数据集 |
| 首轮 neg_control | 884 | GOOD 689 / NCF 101 / WEAK 69 / ORACLE_FAIL 25——发现 101 个 NCF 实为工具 bug 误报 |
| 修复 stub 后重跑 | 779 | NCF 重跑 → 90 GOOD + 11 WEAK;合并得 779 个"GOOD" |
| **修复 classify 后全量复验** | **712** | **712/712 全部 GOOD(100%)**,无 WEAK / NCF / ORACLE_FAIL |

**最终数据集 = 712 个经两轮验证(答案注入 + 负控制)的 GOOD 任务**——剔除了 59 个 WEAK(测试没验证函数)和 8 个真 NCF(空实现无法编译的边界情况)。

WEAK 典型(为何任务会"假验证"):同源多个函数指向同一个无关测试(如 `usb_dc_ep_*` → `test_usb_dc_api`)、`if (IS_ENABLED(CONFIG_X))` 死代码里的调用被关联(如 `bt_enable` → settings 性能测试,CONFIG_BT 未启用)、头文件内联函数(`include/zephyr/...`)关联测试不对。

**验证中发现并修复的工具缺陷**:

| 缺陷 | 影响 | 修复 |
|---|---|---|
| `build_stub_text` 注入漏 `source[:a]`,删掉整个文件头(include/前置函数) | 101 个任务误判 NEG_COMPILE_FAIL | `source[:a] + sig + stub + ...` |
| 旧 stub 一律 `return (类型){0}`,对"返回 0=成功"的 API 误判 WEAK | `bt_enable` 等误判 | 指针→NULL、有符号→-1、无符号→0 |
| `run_test` 只返回 stdout 末尾 300 字符、不含 stderr | 编译错误漏判 | 返回完整 stdout+stderr |
| `classify` 用 `"error:"` 判编译失败,误匹配 ZTEST 断言消息(`error: (rc not equal to ...)`) | 真 GOOD 误判 NEG_COMPILE_FAIL | 先看 `PROJECT EXECUTION` 标记判断测试是否真正执行 |

### 结果分析

```bash
python analyze_results.py     # 汇总统计
python final_analysis.py      # 失败 case 逐例复盘
```

## 配置

### API 配置(settings.json)

基准测试的 API Key/模型放在 `zephyr-claude/settings.json`(已被 `.gitignore` 排除),查找优先级见 `config.py` 的 `_default_claude_settings()`;也可用 `ZEPHYR_CLAUDE_SETTINGS` 环境变量强制指定。agent.py 会把这个文件复制进容器内 Claude 的 `claude_home/settings.json`,并注入 `ANTHROPIC_*` 环境变量。

```bash
cp ../claude_settings.template settings.json   # 然后填入真实 Key/模型
```

### 环境变量

| 变量 | 作用 |
|---|---|
| `ZEPHYR_CLAUDE_SETTINGS` | 指定 API 配置文件 |
| `ZEPHYR_CLAUDE_NETWORK` | Docker 网络模式(默认 `host`) |
| `ZEPHYR_SDK_INSTALL_DIR` | Zephyr SDK 路径(默认 `$HOME/zephyr-sdk-1.0.1`) |
| `ZEPHYR_WORKSPACE_BASE` | 任务工作区目录(默认 `$HOME/.zephyr-workspaces`) |

### 结果字段(results.jsonl)

```json
{
  "task_id": "1",
  "passed": true,
  "claude_completed": true,
  "runtime_s": 45.2,
  "illegal_changes": [],
  "file_tampering": [],
  "error": null,
  "error_category": "none",
  "patch": "...",
  "snapshot": { "git_head": "...", "docker_image_id": "...", "claude_model": "...", "task": {...} }
}
```

## 环境依赖

| 依赖 | 说明 |
|---|---|
| **Docker** | 沙箱运行 Claude Code CLI,需先 `docker build -t zephyr-sandbox:latest .` |
| **tree-sitter** | `pip install tree-sitter tree-sitter-c`(mask.py 需要) |
| **Zephyr SDK** | `~/zephyr-sdk-1.0.1`,west build 需要 |
| **west + 工具链** | 验证阶段编译需要(在 WSL 里 `source ../activate.sh` 配置) |
| **Zephyr 源码** | `../zephyr`,由 `west update` 拉取 |
| **settings.json** | 被测模型的 API Key/模型 |

> ⚠️ `west build` 验证与 `inject_test`/`neg_control` 都需要完整 Zephyr SDK + 工具链,应在 WSL 环境运行。`runner.py --check` 会逐项自检这些依赖。
