"""
Git 操作工具 —— 在每个 task 前后重置 Zephyr 仓库状态。

排除目录: zephyr-claude/, zephyr-bench/, .claude_home/, build/
"""
import subprocess


def hard_reset_repo(repo_root, clean=False):
    """将 git 仓库重置到干净状态。

    Args:
        repo_root: Zephyr 仓库路径
        clean: 是否同时执行 git clean -fd
    """
    r = subprocess.run(
        ["git", "reset", "--hard", "HEAD"],
        cwd=repo_root,
        capture_output=True,
    )
    if r.returncode != 0:
        err = (r.stderr or r.stdout or b"").decode(errors="replace").strip()
        raise RuntimeError(f"git reset --hard failed: {err}")

    if clean:
        r = subprocess.run(
            ["git", "clean", "-fd",
             "-e", "zephyr-claude",
             "-e", "zephyr-bench",
             "-e", ".claude_home",
             "-e", "build"],
            cwd=repo_root,
            capture_output=True,
        )
        if r.returncode != 0:
            err = (r.stderr or r.stdout or b"").decode(errors="replace").strip()
            raise RuntimeError(f"git clean -fd failed: {err}")
