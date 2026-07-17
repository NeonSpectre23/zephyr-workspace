"""
Zephyr-Claude 全局配置

适配自 riot-claude/config.py，路径和构建系统改为 Zephyr。
"""
import os
from pathlib import Path

# 本文件所在目录 = zephyr-claude/
SCRIPT_DIR = Path(__file__).resolve().parent
# Zephyr workspace 根目录 = zephyr-claude/ 的父目录
WORKSPACE_ROOT = SCRIPT_DIR.parent
# Zephyr 源码
ZEPHYR_BASE = WORKSPACE_ROOT / "zephyr"
# zephyr-bench（数据集 + oracle）
BENCH_ROOT = WORKSPACE_ROOT / "zephyr-bench"
# Zephyr SDK
ZEPHYR_SDK_DIR = Path("/home/huyj/zephyr-sdk-1.0.1")

TEST_TIMEOUT = 300              # west build 超时（秒）
WATCHDOG_TIMEOUT = 8 * 3600     # Claude 保险丝（秒）
MAX_OUTPUT_LENGTH = 2_000_000   # 轨迹日志截断阈值

# ---- 目录 ----
TRAJECTORY_DIR = SCRIPT_DIR / "trajectory"
RESULTS_DIR = SCRIPT_DIR / "results"

# ---- Docker 沙箱镜像 ----
DOCKER_IMAGE = "zephyr-sandbox:latest"

# ---- Workspace ----
WORKSPACE_BASE = Path(os.environ.get(
    "ZEPHYR_WORKSPACE_BASE",
    str(Path.home() / ".zephyr-workspaces"),
))

# ---- Claude settings ----
def _default_claude_settings():
    home = Path.home()
    if os.name == "posix":
        users_dir = Path("/mnt/c/Users")
        if users_dir.exists():
            for user_dir in sorted(users_dir.iterdir()):
                candidate = user_dir / ".claude" / "settings.json"
                if candidate.exists():
                    return str(candidate)
        local = home / ".claude" / "settings.json"
        if local.exists():
            return str(local)
    return str(home / ".claude" / "settings.json")

CLAUDE_SETTINGS_SRC = Path(os.environ.get(
    "ZEPHYR_CLAUDE_SETTINGS",
    _default_claude_settings(),
))

CLAUDE_NETWORK_MODE = os.environ.get("ZEPHYR_CLAUDE_NETWORK", "host")

# ---- Docker 资源限制 ----
DOCKER_MEMORY = "4g"
DOCKER_CPUS = "2"
DOCKER_PIDS_LIMIT = "0"

# ---- 数据集默认文件 ----
DEFAULT_TASK_FILES = [
    str(BENCH_ROOT / "zephyr_tasks.c.jsonl"),
    str(BENCH_ROOT / "zephyr_tasks.h.jsonl"),
]
