# Zephyr RTOS Code Completion Benchmark

A benchmark for evaluating LLMs (Claude / DeepSeek / etc.) on **Zephyr RTOS C function completion** — the model sees a Zephyr source file with a function body replaced by `/* MASKED */`, must implement it, and the result is validated by `west build` + running the actual unit tests.

> Forked from a RIOT-OS benchmark pattern. Adapted for Zephyr 4.4.0, tested with DeepSeek v4 Flash via Anthropic-compatible API.
> 
> **Note:** This repo contains only the benchmark framework and dataset. The Zephyr RTOS source code and its modules are **not included** — they are fetched automatically by `west init` + `west update` during setup.

## What's in This Repo (vs. fetched by west)

| Included in git | Fetched by `west update` |
|---|---|
| `zephyr-claude/` — benchmark framework | `zephyr/` — Zephyr RTOS source |
| `zephyr-bench/` — 885 tasks + oracles | `modules/` — HAL, crypto, filesystem |
| `activate.sh`, `setup.sh`, `environment.yml` | `bootloader/` — MCUboot |
| `claude_settings.template`, `.gitignore` | `tools/edtt/` — Bluetooth test tool |
| `README.md`, `.github/workflows/ci.yml` | |

### File tree

```
zephyr-workspace/
├── zephyr/               ← Zephyr RTOS v4.4.0 (fetched by west, not in git)
├── zephyr-bench/         ← Benchmark dataset (885 tasks) + build scripts  ★ in git
│   ├── zephyr_tasks.c.jsonl    628 tasks (C files)
│   ├── zephyr_tasks.h.jsonl    257 tasks (header files)
│   ├── oracles/                885 verified reference implementations
│   ├── build_zephyr_dataset.py Dataset construction from unit tests
│   ├── extract_zephyr_tests.py ZTEST block extraction
│   └── verify_zephyr_tests.py  Oracle injection & validation
├── zephyr-claude/        ← Evaluation harness  ★ in git
│   ├── agent.py               Main pipeline: mask → Claude → verify
│   ├── mask.py                tree-sitter AST operations
│   ├── runner.py              CLI entry point
│   ├── prompt.py              LLM prompt template
│   ├── config.py              Global configuration
│   ├── Dockerfile             Sandbox image (Ubuntu + west + Claude Code CLI)
│   ├── analyze_results.py     Results analysis & statistics
│   └── final_analysis.py      Deep-dive failure pattern analysis
├── bootloader/           ← MCUboot (fetched by west, not in git)
├── modules/              ← HAL, crypto, etc. (fetched by west, not in git)
├── tools/                ← (fetched by west, not in git)
├── activate.sh           ← Environment activation script
├── .gitignore
├── environment.yml       ← Conda environment specification
├── claude_settings.template  ← Claude API config template
├── setup.sh              ← One-click environment setup
└── .github/workflows/    ← CI configuration
```

## How It Works

```
1. git reset --hard HEAD          # Clean the Zephyr source
2. Copy target file → workspace   # Only the file to edit
3. tree-sitter: mask function body → /* MASKED */
4. Run Claude Code CLI in Docker  # Zephyr source RO, target file RW
5. Read Claude output → diff
6. AST integrity check            # Did Claude edit only the target function?
7. Inject Claude code → original source
8. west build + run unit test     # Compiles? Tests pass?
9. git reset → restore            # Back to clean state
```

### Safety Checks

- **AST normalization equality**: compares normalized masked vs. final source — catches edits outside the target function
- **Body keyword check**: forbids `#define`, `#include`, `typedef`, `__asm__` inside function body
- **Docker sandbox**: `--cap-drop ALL`, `--security-opt no-new-privileges`, read-only mount for Zephyr source

## Prerequisites

| Dependency | Version / Location | Notes |
|---|---|---|
| **Zephyr SDK** | [v1.0.1](https://github.com/zephyrproject-rtos/sdk-ng/releases/tag/v1.0.1) | GNU cross-compiler toolchains |
| **Conda / Miniconda** | Python 3.12 | Build tool isolation |
| **Docker** | Desktop 24+ | Sandbox for Claude Code CLI |
| **Claude Code CLI** | v2.1.146 (in Docker) | LLM under test |
| **API Key** | Anthropic / compatible | e.g. DeepSeek, Anthropic |

## Setup

### 1. Clone this repo

```bash
git clone https://github.com/YOUR_USER/zephyr-workspace.git
cd zephyr-workspace
```

### 2. Fetch Zephyr source code + modules

The Zephyr RTOS source and its modules are not stored in this repo (they are large and version-pinned). Restore them with west:

```bash
# Clean out any old west configuration
rm -rf .west

# Initialize west with the official Zephyr manifest at v4.4.0
west init -m https://github.com/zephyrproject-rtos/zephyr --mr v4.4.0

# Fetch all projects (zephyr/, modules/, bootloader/, tools/)
west update
```

> ⏱ This downloads ~1 GB of source code. Time for coffee.

### 3. Set up conda environment

```bash
conda env create -f environment.yml
conda activate zephyr
```

Or use the activation script (handles conda activation + env vars):

```bash
source activate.sh
```

### 4. Install Zephyr SDK

```bash
wget https://github.com/zephyrproject-rtos/sdk-ng/releases/download/v1.0.1/zephyr-sdk-1.0.1_linux-x86_64.tar.xz
tar xf zephyr-sdk-1.0.1_linux-x86_64.tar.xz -C ~/
cd ~/zephyr-sdk-1.0.1/
./setup.sh
```

### 5. Build Docker sandbox

```bash
cd zephyr-claude
docker build -t zephyr-sandbox:latest .
```

### 6. Configure API key

Copy the template and fill in your API key:

```bash
mkdir -p ~/.claude
cp claude_settings.template ~/.claude/settings.json
# Edit ~/.claude/settings.json → replace "sk-xxx" with your real key
```

Supported API providers (via `ANTHROPIC_BASE_URL`):
- **DeepSeek**: `https://api.deepseek.com/anthropic`
- **Anthropic**: `https://api.anthropic.com` (omit or set to empty)
- **Any Anthropic-compatible proxy**

Start with the default model `deepseek-v4-flash` — change `ANTHROPIC_MODEL` as needed.

### 7. Verify dependencies

```bash
cd zephyr-claude
python runner.py --check
```

Expected output:
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

## Usage

### Run a single task

```bash
cd zephyr-claude
python runner.py --task-id 1
```

### Run a batch

```bash
python runner.py --batch 10
```

### Resume (skip already-passed tasks)

```bash
python runner.py --batch 100 --resume
```

### Start from a specific task

```bash
python runner.py --batch 50 --start 200
```

### Verify the dataset (inject oracle → build → test)

```bash
python inject_test.py --batch 5
```

### Analyze results

```bash
python analyze_results.py
```

## Dataset: 885 Tasks

Tasks are extracted from Zephyr RTOS unit tests (`tests/unit/`, `tests/subsys/`, `tests/lib/`) via `extract_zephyr_tests.py`:

| Source | Count | Priority |
|---|---|---|
| `lib/` | highest | Core library functions |
| `subsys/` | high | Subsystem APIs |
| `arch/` | medium | Architecture-specific |
| `kernel/` | medium | Kernel APIs |
| `drivers/` | low | Device drivers |

Each task contains:
- `task_id` — unique identifier
- `sut_function` — function name to implement
- `source_path` — file containing the function
- `masked_code` — function signature with `/* TODO(agent) */` body
- `oracle` — path to reference implementation
- `run_command` — `west build` command to verify
- `unit_test` — ZTEST name for test pass/fail detection

## Error Categories

| Category | Meaning | Common Causes |
|---|---|---|
| `compile_error` | Build failure | Wrong API name, preprocessor mismatch, syntax |
| `test_failure` | Compiles but test fails | Algorithm edge cases, wrong constants |
| `crash` | Runtime crash | NULL pointer, uninitialized fields |
| `illegal_modifications` | Edited outside target function | `#undef`, global changes |
| `timeout` | `west build` exceeded 5 min | Large dependency tree |
| `watchdog` | Claude session timeout (8h) | Stuck / infinite loop |

## Results

Results are stored in `zephyr-claude/results/results.jsonl` (JSONL format, one result per line):

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

Trajectory logs (full Claude session + diff + verify output) are stored in `zephyr-claude/trajectory/`.

## License

Zephyr RTOS is under [Apache 2.0](zephyr/LICENSE).  
Benchmark tooling is provided under the same terms.
