# OSKernelBench 修订——给 Yujie 的 PR#3 整改清单（Zephyr track onboarding）

> 本文是执笔方（下游分析）代项目负责人，就 **PR#3（导入 Zephyr）** 给出的一次性整改清单。
> PR#3 把 Zephyr 作为**第三个完整开源 RTOS 数据集**（885 任务，与 RIOT 平级）引入，框架完整、隔离设计用心，是有价值的贡献。但本轮 IEEE TSE 重投**暂不纳入 Zephyr**（决策见 [`docs/adr/0001-defer-zephyr-from-round-1.md`](docs/adr/0001-defer-zephyr-from-round-1.md)），原因是 PR#3 目前尚未达到纳入正文的 **onboarding standard**。
> 本文 = 重新激活 Zephyr 的**达标门槛（onboarding standard 四条）** + PR#3 **四个具体硬伤**的整改指引。
> **达标前 PR#3 不合并进 `main`、不投入算力跑 885 全量**——先把框架修对，避免在质量不达标时白跑（这是为你省算力，见第三节）。达标即可作为第三条开源 track 纳入未来轮次。
> 所有代码引用基于协作仓 `pr3` 分支（head `471bdeb8`，2026-07-17）逐文件核实，标注 `文件:行号`。

---

## 0. 先说结论（TL;DR）

**这份清单不是拒绝，是一张"怎样才能纳入"的路线图。**

- **值得肯定的**：885 任务（628 `.c` + 257 `.h`，`oracles/` 628 个 `.c` 已对齐）规模高于 RIOT 538；`zephyr-claude` 框架照搬 `riot-claude`、完整对称（runner / agent / mask / prompt / Dockerfile / analyze_results / final_analysis）；容器隔离在**多数维度上比 RIOT 还规范**——`--cap-drop ALL` + `--security-opt no-new-privileges` + `--pids-limit` + `--memory`/`--cpus` 限额 + 整个源码树 `:ro` 只读挂载 + 仅 `target.c` 可写 + `tmpfs`（`agent.py:241-261`）；`Dockerfile` 干净（ubuntu 24.04 + 非 root `agent` 用户 + 固定 `claude-code@2.1.146`），**还可复用给闭源 QSemOS 补 Docker 隔离**。
- **暂存的四个硬伤**：① 零评估结果（PR 内无 `results/`/`trajectory/`/`whitebox`）；② 隔离漏洞（网络默认 `host` 而非 `none`）；③ 硬编码本地路径（`/home/huyj/...`，别人跑不了）；④ 失败分类是浅分类（未对齐机制三大类）。
- **激活路径**：满足下面 **onboarding standard 四条** → 达标即纳入。本轮规模关（审稿点要求 >720 任务）已由 RIOT 538 + QSemOS 240 = 778 达成，**不依赖 Zephyr**，所以**本轮不催赶**，请按你的节奏达标。

---

## 一、Onboarding standard 四条（纳入正文的硬门槛）

> 一条新 track（或被暂存的 track 重新激活）纳入 OSKernelBench 正文，必须**同时**满足以下四条（权威定义见 `CONTEXT.md` › Track Onboarding）。任一条未达，该 track 不进正文、对应 PR 不合并进 `main`。

1. **隔离**：Docker `--network none` + **无硬编码本地路径**（换一台机器可复现）。→ 对应硬伤 ②③
2. **失败分类**：对齐**机制三大类**（幻觉 / 理解偏差 / 指令遵循失败），**不得用浅分类**。→ 对应硬伤 ④
3. **模型与数据**：**DS-v4-pro 与 GLM-5.1 全量结果** + 结构化 `results.jsonl` + `whitebox_report.json` 齐全，schema 与既有 track（`riot-claude`/`qsem-claude`）一致。→ 对应硬伤 ①
4. **口径对称**：模型矩阵、通过率口径、完整性判定与既有 track 完全一致（细则见第四节）。→ 贯穿全部

---

## 二、PR#3 四个具体硬伤（逐条：现状 / 为何是硬伤 / 请你做什么 / 如何验证）

### 硬伤① 零评估结果（对应门槛 3）

**现状**（✅已核实）：`pr3` 分支 `Open_source/zephyr-workspace/` 下只有数据集定义（`zephyr-bench/zephyr_tasks.c.jsonl` 628 行、`zephyr_tasks.h.jsonl` 257 行、`zephyr_tests.jsonl`），**没有** `results/`、`trajectory/`、`whitebox_report.json` 任何一份。但 `zephyr-claude/final_analysis.py` 里已写死具体失败分析（undeclared 2 / undefined reference 5 / syntax 3 / CRC 写错 28 / segfault 空指针 16 / ILLEGAL 10，共 64 例，还带 "flash NULL 指针"、"ninja/cmark 输出夹二进制字符解析失败" 等逐例细节）——**说明本地已经跑过并做过详细失败分析，只是产物没进 PR**。

**为什么是硬伤**：正文引用任何 Zephyr 数字，都必须有**可复现的结构化结果**背书。"有数据集却无结果"会被审稿人直接质疑（这也是本轮暂存的首要原因）。

**请你做什么**：
- **(a) 先把本地已有的运行产物提交到 PR**：`trajectory/` 日志 + 汇总 + `final_analysis.py` 所依据的原始 `results`。这一步不烧新算力，只是把已有的东西入库，让我们了解现状口径。
- **(b) 待框架三硬伤（②③④）修好、口径对齐后，再跑 DS + GLM 全量 885**，产出结构化 `results.jsonl`（885 × 2 行）+ `whitebox_report.json`，字段 schema 与 `qsem-claude`/`riot-claude` 一致。
- ⚠️ **(a) 与 (b) 的先后很重要**（见第三节整改顺序）：**不要在框架还带着 ②③④ 漏洞时就跑全量**——那样跑出来的口径不对（网络没隔离 / 分类不对齐），结果要作废重跑，白费 885 × 2 次的算力。ADR-0001"达标前不跑 885"正是这个用意，是**为你省算力**，不是拖延。

**如何验证**：`results/{deepseek-v4-pro,glm-5.1}/results.jsonl` 各 885 行 + 对应 `whitebox_report.json` 齐全；字段与既有 track 逐一对齐。

---

### 硬伤② 隔离漏洞：Docker 网络默认 `host`（对应门槛 1）

**现状**（✅已核实）：
```python
# config.py:57
CLAUDE_NETWORK_MODE = os.environ.get("ZEPHYR_CLAUDE_NETWORK", "host")   # ← 默认值就是 host
# agent.py:248-249
if CLAUDE_NETWORK_MODE:
    docker_args.extend(["--network", CLAUDE_NETWORK_MODE])
```
即容器默认走 `--network host`，与宿主共享网络栈。而 RIOT / QSemOS 侧是 `--network none`（不联网原则）。

**为什么是硬伤**：**不联网是评估协议的核心防作弊 / 可复现假设**——被测模型不得在执行期联网查资料、拉取答案或额外依赖。`host` 网络破坏这条假设，也让 Zephyr 无法与既有两 track 同口径对比。

**一个必须点破的设计张力**：你的 Zephyr 源码走 `west init` + `west update` **联网拉取**（`README.md:7` 明确"源码不在 git 内，由 west 在 setup 时拉取"），且验证是在容器内跑 `west build`——`host` 默认很可能是为了让 build 期能联网取依赖。但**正解是把构建依赖预置好**（源码 + SDK + west 模块在容器外准备完，只读挂载或打进镜像），**而不是用 `host` 网络掩盖**。好消息：你的 `agent.py` 已经是"宿主 `west update` 拉好 → 只读挂载 `ZEPHYR_BASE` 进容器（`agent.py:255` `/workspace:ro`）"的架构，方向完全正确，只差把 build 期残余的联网点也离线化。

**请你做什么**：
- (a) `config.py:57` 默认值 `"host"` → `"none"`（与 RIOT 对齐）；
- (b) 在 `--network none` 下跑通一个样本的 `west build`。若失败 → 定位 build 期的联网点，把对应依赖预置进镜像 / 挂载（可复用 `Dockerfile` 已装的 `west`/`pyelftools` 等）；
- (c) 记录"离线可 build"的验证结论。

**如何验证**：`config.py` 默认 `none`；`grep -rn 'host' zephyr-claude/config.py zephyr-claude/agent.py` 无网络相关残留；在断网环境下能跑通样本。

---

### 硬伤③ 硬编码本地路径（对应门槛 1）

**现状**（✅已核实）：
```python
# config.py:18
ZEPHYR_SDK_DIR = Path("/home/huyj/zephyr-sdk-1.0.1")
# inject_test.py:21
SDK_DIR = Path("/home/huyj/zephyr-sdk-1.0.1")
```
两处写死了本机绝对路径，别人 clone 下来直接跑不了（`runner.py:174` 的环境自检会失败）。

**讽刺的对照**：你自己的 `activate.sh:22` / `setup.sh:18` 已经用了 `${HOME}/zephyr-sdk-1.0.1`（相对 `HOME`，可复现）——**shell 层已经做对了，Python 层没跟上**。

**请你做什么**：Python 两处改成"读环境变量 + `HOME` 回退"，与 shell 脚本对齐，例如：
```python
ZEPHYR_SDK_DIR = Path(os.environ.get("ZEPHYR_SDK_INSTALL_DIR", str(Path.home() / "zephyr-sdk-1.0.1")))
```
（`inject_test.py:21` 同理。）

**如何验证**：`grep -rn '/home/huyj' zephyr-claude/` 无命中；换一台机器 / 改 `HOME` 后环境自检通过。

---

### 硬伤④ 浅失败分类未对齐机制三大类（对应门槛 2）

**现状**（✅已核实）：`final_analysis.py` 用的是**运行器级浅分类**——`COMPILE_ERROR`（undeclared / undefined reference / syntax）/ `TEST_FAILURE` / `CRASH` / `ILLEGAL`。这是"**报错类型**"，不是"**失败机理**"，未对齐我们本轮定死的机制三大类。

**请你做什么**：像 `qsem-claude`/`riot-claude` 一样跑 **whitebox 三级分类**（`classification.{L1,L2,...}` + `illegal_changes` + `module`），产出 `whitebox_report.json`，再按第四节的映射归到三大类。下表是你现有浅类到三大类的**建议映射**（供参照，最终以 whitebox `L2` 逐例判定为准）：

| 你的浅类（final_analysis） | 建议机制大类 | 说明 |
|---|---|---|
| `undefined reference`（调用不存在的函数） | **幻觉** | 编造了不存在的符号 / API |
| `undeclared`（被预处理器删掉）、`syntax`（打破 `#if/#endif` 配对） | 多为**理解偏差** | 对条件编译 / 宏的理解错；个别"凭空造符号"归幻觉，逐例判 |
| `TEST_FAILURE`（如 CRC 查表算法写错） | **理解偏差** | 逻辑 / 计算偏差（`wrong_calculation`）|
| `CRASH`（segfault，访问空指针） | **理解偏差** | 缺空指针检查（`missing_null_check`）|
| `ILLEGAL`（函数体外加 `#undef` 等越界编辑） | **指令遵循失败** | `illegal_changes` 非空 → **最先判定**（见第四节铁律）|

**如何验证**：`whitebox_report.json` 的每条 `L2` 能按第四节映射到三大类之一；`illegal_changes` 非空的 case 与 whitebox 记录不相交（判定顺序落实的标志）。

---

## 三、整改顺序建议（先框架、后算力——呼应 ADR-0001）

```
第 1 步：修 ②③④          （纯改代码 / 脚本，不烧算力）
             │
第 2 步：跑 DS + GLM 全量 885   （框架达标后才做，即硬伤①(b)；产出 results.jsonl + whitebox）
             │
第 3 步：口径对称自查       （第四节铁律）
             ▼
        达标 → 可纳入未来轮次
```

**三步全部完成前，PR#3 不合并进 `main`。** 这个顺序的唯一目的是**避免在框架还不达标时就跑全量、结果作废重跑**。硬伤①(a)（把本地已有产物先入 PR）可以马上做，不受此顺序约束。

---

## 四、Failure Taxonomy 权威定义（请让 Zephyr 的 whitebox 与此对齐）

> 本节与《给 Zibin 的对齐清单（定稿版）》第三节**同一版本**——三个 track 共用一套失败分类语言，保证跨 track 可比。

### 机制维度（mechanism）—— 三大类，**互斥且覆盖全部失败**

| # | 类别 | 定义 | 判定依据 |
|---|---|---|---|
| 1 | **指令遵循失败** instruction-following | 越界编辑 / 完整性违规（编辑目标函数体以外，或插入禁用指令）| `results.jsonl` 的 `illegal_changes` 非空 → **最先判定** |
| 2 | **幻觉** hallucination | 编译失败，编造不存在的符号 / API / 类型 | whitebox `L2 ∈ {undefined_reference, hallucinated_api, hallucinated_api_and_wrong_architecture, implicit_declaration}` |
| 3 | **理解偏差** misunderstanding | 编译通过但单测失败 / 运行崩溃；逻辑、边界、同步、数据结构偏差 | whitebox `L2 ∈ {missing_conditional_guard, extra_conditional_guard, wrong_calculation, missing_synchronization, wrong_error_handling, data_structure_misuse, semantic_drift, missing_null_check, ...}` |

**判定顺序（铁律，三类互斥）**：
```
对每条失败 case：
  1. 若 is_false_failure（L2 = harness_bug_*）→ 整条剔除，不计入失败；
  2. 否则若 illegal_changes 非空          → 指令遵循失败；
  3. 否则按 L2 映射                        → 幻觉 或 理解偏差。
```

### whitebox 字段对齐说明

| 论文概念 | whitebox / results 字段 | 说明 |
|---|---|---|
| 机制三大类 | 由 `classification.L2`（经上表映射）+ `illegal_changes` 组合得到 | **不是**直接读 `classification.L1` |
| `classification.L1` | = runner 的 error_category（compile_error / crash / test_failure / false_failure_detection）| 这是**运行器判定**，不是机制类；仅用于回溯 |
| `classification.L2` | 细粒度子类 | 经 `L2_TO_MECH` 映射到三大类（映射表见 `analysis/failure_crosstab.py`）|
| `module` | OS 模块维度 | L1 聚合取模块路径前一段（Zephyr 的划分见下）|
| `illegal_changes` | 指令遵循失败的**唯一**来源（在 `results.jsonl`，不在 whitebox）| 须与 whitebox 不相交 |
| `is_false_failure` | harness 误判标记 | 剔除，不计入失败 |

### 双标签（dual-label）

每条失败 case 同时携带「机制类」+「OS 模块类」两个**平行**标签（同一 case 可同属两个维度）；两维度**不交叉**——分别呈现为两张独立的单维度图（机制分类树 L1→L2 + 模块树），每图内 OS × 模型 **per-model 对号入座**，**不跨模型相加**。

### OS 模块维度（Zephyr 请给出自己的 L1 划分）

- 既有 track 的 L1 模块集：QSem = `{ipc, kernel, perf, utility, extend, other}`；RIOT 体系不同（`gnrc_*` 网络栈 / `sys/*` / `core/*`，由 Zibin 侧确定）。
- **Zephyr 模块体系又不同**（如 `kernel/`、`drivers/`、`lib/`、`subsys/`、`arch/` …）。请你**给出 Zephyr 的模块 L1 划分**，并在 whitebox 里实现，使三 track 的 L1 集合可对齐（或显式说明哪些项不可对齐）。⚠️ 别照搬 QSem 的 `classify_module` 关键字，Zephyr 目录结构对不上。

#### Zephyr 模块 L1 划分（已落实）

**L1 集合**：`{kernel, subsys, lib, drivers, arch, <subsystem>, modules, samples, other}`，由 SUT `source_path` 顶层目录派生；`include/zephyr/<sub>/...` 的头文件内联函数映射到其子系统 `<sub>`（与 `subsys/<sub>/` 归为一类）。

| L1 | source_path 前缀 | 说明 | 与 QSem / RIOT 对齐性 |
|---|---|---|---|
| `kernel` | `kernel/` | 内核核心 API | 概念对齐 QSem `kernel` |
| `subsys` | `subsys/` | 子系统（网络/蓝牙/日志/USB/存储/…），**不再细分** | 概念对齐 RIOT `sys/*`（粒度更粗） |
| `lib` | `lib/` | 库（标准库/hex/heap/uuid/…） | 无直接对应，列为独立 L1 |
| `drivers` | `drivers/` | 设备驱动 | 概念对齐（RIOT 驱动分散于多个模块） |
| `arch` | `arch/` | 架构相关 | 无直接对应 |
| `<subsystem>` | `include/zephyr/<sub>/...` | 头文件内联函数，归入其子系统 | **Zephyr 特有**（RIOT 无 include 层），显式不可对齐 |
| `modules` | `modules/` | west 外部模块 | 无直接对应 |
| `samples` | `samples/` | 示例代码 | 无直接对应 |
| `other` | 其它 | 兜底 | — |

> ⚠️ `subsys/` 下的子系统（`net`、`bluetooth`、`logging`…）与 `include/zephyr/<sub>/` 归并后，L1 会出现子系统级条目（如 `net`、`sys`）。若要更细，可将 `subsys/<sub>/` 也展开为 `<sub>`——但当前统一采用"顶层目录 + include 归并"，粒度居中、可复现。

**实现位置**：`zephyr-claude/whitebox.py` 的 `classify_module()`。每条失败 case 的 `module` 字段即此 L1；机制三大类由 `mechanism` 字段给出（`hallucination` / `misunderstanding` / `false_failure`）。两者构成**双标签**（同一 case 同属机制维 + 模块维），互不交叉。

**对齐说明**：机制维度完全共用第三节/四节的权威 taxonomy（`L2_CONSOLIDATED` + `L2_TO_MECH` 已按参考实现对齐 QSem/RIOT）。模块维度上，`kernel`/`drivers`/`subsys` 可概念对齐，`lib`/`arch`/`modules`/`samples` 为 Zephyr 独有、RIOT/QSem 无对应项——按本节要求**显式声明为不可对齐项**，在跨 track 模块图里只做 per-model 呈现、不跨 track 相加。

---

## 五、交付清单（分两阶段）

**阶段一 · 框架（先做，不烧算力）**
- [ ] 硬伤②：`config.py:57` 默认 `network = none` + 断网跑通样本 `west build`
- [ ] 硬伤③：`config.py:18` / `inject_test.py:21` 去硬编码，改 env + `HOME` 回退
- [ ] 硬伤④：whitebox 三级分类脚本（对齐机制三大类）+ **Zephyr 模块 L1 划分**
- [ ] 硬伤①(a)：本地**已有**运行产物先入 PR（trajectory + 汇总 + 原始 results）

**阶段二 · 算力（框架达标后）**
- [ ] 硬伤①(b)：DS-v4-pro + GLM-5.1 **全量 885**，`results.jsonl`（885 × 2）+ `whitebox_report.json`（schema 同 `qsem-claude`/`riot-claude`）
- [ ] 口径对称自查（第四节铁律：判定顺序、`illegal_changes` 为指令遵循唯一来源、完整性违规统一算失败、per-model 不相加、Zephyr 模块 L1 与既有 track 对齐）

---

## 附：几点说明

- **达标即激活**：满足 onboarding standard 四条后，Zephyr 即可作为第三条开源 track 纳入（未来轮次），届时正文的模型矩阵 / 通过率表 / 双标签两图会扩到三 track。你的框架质量高，工作不会浪费——`Dockerfile` 与隔离设计还计划复用给闭源 QSemOS。
- **本轮不催赶**：本轮规模关已达成，Zephyr 不进本轮不影响投稿，请按你的节奏推进达标。
- **称呼约定**：本文以「项目负责人 / 执笔方 / 第一作者」指代相关角色，不涉个人姓名。
- **跟进渠道**：本清单建议贴在 **PR#3 的 comment** 就地跟进（与既有讨论同上下文）；如需完整背景，可参阅仓内 `CONTEXT.md`（术语）与 `docs/adr/0001-defer-zephyr-from-round-1.md`（暂存决策）。

*附：本文代码事实由执笔方于 2026-07-19 对协作仓 `pr3` 分支（head `471bdeb8`）逐文件核实；口径与 Failure Taxonomy 定义与《给 Zibin 的对齐清单（定稿版）》保持一致。*
