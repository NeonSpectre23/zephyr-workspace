"""
Zephyr-Claude Agent Pipeline — 零复制版。

Claude 容器挂载 zephyr/ 为只读，仅目标文件可写。
验证直接在原始源码上跑 west build，用 git reset 恢复。
"""
import difflib
import hashlib
import json
import os
import re
import selectors
import shutil
import subprocess
import sys
import time
from pathlib import Path

from config import *
from prompt import TASK_PROMPT
from mask import (apply_mask, check_ast_integrity, check_body_integrity,
                  extract_function_body)
from git_utils import hard_reset_repo
from logger import append_trajectory

ERROR_NONE = "none"
ERROR_ILLEGAL = "illegal_modifications"
ERROR_COMPILE = "compile_error"
ERROR_CRASH = "crash"
ERROR_TEST_FAIL = "test_failure"
ERROR_TEST_NOT_RUN = "test_not_executed"
ERROR_TIMEOUT = "timeout"
ERROR_WATCHDOG = "watchdog"
ERROR_EXCEPTION = "exception"
ERROR_API = "api_error"

_ANSI_RE = re.compile(r'\x1B(?:[@-Z\\-_]|\[[0-?]*[ -/]*[@-~])')

def strip_ansi(text):
    return _ANSI_RE.sub('', text)

def _docker_user_args():
    return ["--user", f"{os.getuid()}:{os.getgid()}"]


class ClaudeAgent:

    def __init__(self, workspace_root):
        self.ws_root = Path(workspace_root)

    # ═══════════════════════════════════════════════════════════════
    # 主入口
    # ═══════════════════════════════════════════════════════════════

    def run(self, task, dataset_paths=None):
        start = time.time()
        workspace_dir = None

        try:
            hard_reset_repo(ZEPHYR_BASE, clean=True)

            # Step 2: 只复制目标文件，mask
            workspace_dir, masked_source = self._prepare_workspace(task)

            # Step 3: 快照
            snapshot = self._capture_snapshot(task, ZEPHYR_BASE, dataset_paths)

            # Step 5: Claude（容器挂载 zephyr/ ro，仅目标文件 rw）
            claude_completed, claude_output = self._run_claude(task, workspace_dir)
            watchdog_kill = "[WATCHDOG]" in (claude_output or "")

            # Step 7: 读 Claude 输出
            final_source = (workspace_dir / "target.c").read_text()

            # Step 8: diff
            diff = "".join(difflib.unified_diff(
                masked_source.splitlines(keepends=True),
                final_source.splitlines(keepends=True),
                fromfile=task["source_path"], tofile=task["source_path"],
            ))

            # Step 9: AST 校验
            integrity_ok = check_ast_integrity(
                masked_source, final_source, task["sut_function"])
            if integrity_ok:
                try:
                    body = extract_function_body(final_source, task["sut_function"])
                except Exception:
                    body = None
                if body is not None:
                    integrity_ok = check_body_integrity(body)

            illegal = []
            if not integrity_ok:
                illegal.append("integrity violation")

            self._record_trajectory(task, claude_output, diff, illegal)

            if illegal:
                return self._make_result(task, start, passed=False,
                    claude_completed=claude_completed,
                    illegal=illegal, error="illegal_modifications",
                    error_category=ERROR_ILLEGAL, diff=diff, snapshot=snapshot)

            # Step 10: 注入 Claude 结果到原始源码，west build 验证
            tests_passed, test_output, total_failures, test_error_cat = (
                self._verify_tests(task, workspace_dir))

            append_trajectory(
                TRAJECTORY_DIR / f"task_{task['task_id']}.log",
                f"\n{'='*60}\n=== Harness Verify ===\n{'='*60}\n{test_output}\n")

            test_error = None
            test_cat = test_error_cat
            if watchdog_kill:
                test_error = "watchdog_killed_claude"
                test_cat = ERROR_WATCHDOG
            elif not claude_completed and tests_passed:
                test_error = "claude_did_not_complete"
                test_cat = ERROR_API
            elif not tests_passed:
                test_error = test_error_cat or "test_failure"

            return self._make_result(task, start, passed=tests_passed,
                claude_completed=claude_completed, illegal=illegal,
                error=test_error, error_category=test_cat, diff=diff,
                snapshot=snapshot, total_test_failures=total_failures)

        finally:
            if workspace_dir and workspace_dir.exists():
                try:
                    shutil.rmtree(workspace_dir)
                except Exception:
                    pass
            hard_reset_repo(ZEPHYR_BASE, clean=True)

    # ═══════════════════════════════════════════════════════════════
    # 工作区准备（只复制目标文件）
    # ═══════════════════════════════════════════════════════════════

    def _prepare_workspace(self, task):
        ws_dir = WORKSPACE_BASE / f"task_{task['task_id']}"
        claude_home = ws_dir / "claude_home"

        if ws_dir.exists():
            shutil.rmtree(ws_dir)
        ws_dir.mkdir(parents=True)
        claude_home.mkdir()

        # 只复制目标文件
        target_src = ZEPHYR_BASE / task["source_path"]
        target_dst = ws_dir / "target.c"
        target_dst.parent.mkdir(parents=True, exist_ok=True)
        shutil.copy2(target_src, target_dst)

        # Mask
        apply_mask(target_dst, task["sut_function"], task["masked_code"])
        masked = target_dst.read_text()
        (ws_dir / "masked_source.txt").write_text(masked)

        # Claude settings
        if CLAUDE_SETTINGS_SRC.exists():
            settings = json.loads(CLAUDE_SETTINGS_SRC.read_text())
            settings.setdefault("hooks", {})["claude-on-completion"] = (
                "echo '=== ZEPHYR_TASK_COMPLETED ==='")
            (claude_home / "settings.json").write_text(
                json.dumps(settings, indent=2, ensure_ascii=False))

        return ws_dir, masked

    # ═══════════════════════════════════════════════════════════════
    # 环境快照
    # ═══════════════════════════════════════════════════════════════

    def _capture_snapshot(self, task, repo_dir, dataset_paths=None):
        snap = {}
        r = subprocess.run(["git", "rev-parse", "HEAD"],
                           capture_output=True, text=True, cwd=ZEPHYR_BASE)
        snap["git_head"] = r.stdout.strip() if r.returncode == 0 else None
        r = subprocess.run(["docker", "inspect", "--format={{index .RepoDigests 0}}",
                            DOCKER_IMAGE], capture_output=True, text=True)
        snap["docker_image_digest"] = r.stdout.strip() if r.returncode == 0 else None
        r = subprocess.run(["docker", "inspect", "--format={{.Id}}",
                            DOCKER_IMAGE], capture_output=True, text=True)
        snap["docker_image_id"] = r.stdout.strip() if r.returncode == 0 else None
        try:
            cfg = json.loads(CLAUDE_SETTINGS_SRC.read_text())
            snap["claude_model"] = cfg.get("model", "unknown")
        except Exception:
            snap["claude_model"] = "unknown"
        r = subprocess.run(["docker", "run", "--rm", "--entrypoint", "claude",
                            DOCKER_IMAGE, "--version"], capture_output=True, text=True)
        snap["claude_code_version"] = r.stdout.strip() if r.returncode == 0 else None
        prompt = self._build_prompt(task)
        snap["prompt_sha256"] = hashlib.sha256(prompt.encode()).hexdigest()
        snap["prompt_template_sha256"] = hashlib.sha256(TASK_PROMPT.encode()).hexdigest()
        snap["prompt_text"] = prompt
        snap["task"] = task
        for lf in ["west.yml", "CMakeLists.txt", "SDK_VERSION"]:
            p = repo_dir / lf
            if p.exists():
                snap[f"{lf}_sha256"] = hashlib.sha256(p.read_bytes()).hexdigest()
        if dataset_paths:
            dataset_hashes = {}
            for dp in sorted(dataset_paths):
                dp = Path(dp)
                if dp.exists():
                    dataset_hashes[str(dp)] = hashlib.sha256(dp.read_bytes()).hexdigest()
            snap["dataset_sha256"] = dataset_hashes
        return snap

    def _build_prompt(self, task):
        return TASK_PROMPT.format(
            sut_function=task["sut_function"],
            source_path=task["source_path"])

    # ═══════════════════════════════════════════════════════════════
    # Claude 容器
    # ═══════════════════════════════════════════════════════════════

    _ALLOWED_CLAUDE_ENV = {
        "ANTHROPIC_API_KEY", "ANTHROPIC_AUTH_TOKEN", "ANTHROPIC_BASE_URL",
        "ANTHROPIC_MODEL",
    }

    @staticmethod
    def _claude_env_args():
        try:
            cfg = json.loads(CLAUDE_SETTINGS_SRC.read_text())
            allowed = ClaudeAgent._ALLOWED_CLAUDE_ENV
            return sum((["-e", f"{k}={v}"] for k, v in cfg.get("env", {}).items()
                       if k in allowed), [])
        except Exception:
            return []

    def _run_claude(self, task, workspace_dir):
        prompt = self._build_prompt(task)
        claude_home = workspace_dir / "claude_home"
        target_path = task["source_path"]

        docker_args = [
            "docker", "run", "--rm", "-i",
            "--init", "--cap-drop", "ALL",
            "--security-opt", "no-new-privileges",
            "--pids-limit", DOCKER_PIDS_LIMIT,
            "--memory", DOCKER_MEMORY, "--cpus", DOCKER_CPUS,
        ]
        if CLAUDE_NETWORK_MODE:
            docker_args.extend(["--network", CLAUDE_NETWORK_MODE])
        docker_args.extend(_docker_user_args())
        docker_args.extend(self._claude_env_args())
        docker_args.extend([
            "-e", "TERM=dumb", "-e", "NO_COLOR=1", "-e", "CI=1",
            # Mount entire zephyr source READ-ONLY
            "-v", f"{ZEPHYR_BASE}:/workspace:ro",
            # Mount ONLY the target file READ-WRITE
            "-v", f"{workspace_dir / 'target.c'}:/workspace/{target_path}:rw",
            "-v", f"{claude_home}:/home/agent/.claude:rw",
            "--tmpfs", "/tmp:size=1g,exec",
            "--tmpfs", "/home/agent/.cache:size=500m",
            "--tmpfs", "/home/agent/.config:size=100m",
            "-w", "/workspace",
            "--entrypoint", "bash", DOCKER_IMAGE,
            "-c", 'claude --dangerously-skip-permissions -p "$(cat)" 2>&1',
        ])

        print(f"  [Claude] {task['sut_function']} (task {task['task_id']})")

        proc = subprocess.Popen(docker_args, stdin=subprocess.PIPE,
                                stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
                                text=True)
        proc.stdin.write(prompt)
        proc.stdin.close()

        sel = selectors.DefaultSelector()
        sel.register(proc.stdout, selectors.EVENT_READ)
        output_chunks = []
        last_activity = time.time()
        claude_completed = False

        while True:
            if proc.poll() is not None:
                break
            if time.time() - last_activity > WATCHDOG_TIMEOUT:
                proc.kill()
                output_chunks.append("\n[WATCHDOG] killed\n")
                break
            events = sel.select(timeout=1.0)
            for key, _ in events:
                line = key.fileobj.readline()
                if not line:
                    break
                output_chunks.append(line)
                last_activity = time.time()
                clean = strip_ansi(line)
                sys.stdout.write(clean)
                sys.stdout.flush()
                if re.search(r"^=== ZEPHYR_TASK_COMPLETED ===$", clean.strip()):
                    claude_completed = True

        sel.unregister(proc.stdout)
        sel.close()
        while True:
            try:
                line = proc.stdout.readline()
            except Exception:
                break
            if not line:
                break
            output_chunks.append(line)
            if re.search(r"^=== ZEPHYR_TASK_COMPLETED ===$", strip_ansi(line).strip()):
                claude_completed = True
        proc.wait(timeout=10)
        full_output = "".join(output_chunks)
        if not claude_completed and proc.returncode == 0:
            claude_completed = True
        print(f"  Claude {'completed' if claude_completed else f'did not complete (rc={proc.returncode})'}")
        return claude_completed, full_output

    # ═══════════════════════════════════════════════════════════════
    # 验证（在原始源码上跑 west build）
    # ═══════════════════════════════════════════════════════════════

    def _verify_tests(self, task, workspace_dir):
        """将 Claude 编辑的文件注入原始源码，west build，然后 git reset 恢复。"""
        target_src = ZEPHYR_BASE / task["source_path"]
        edited_file = workspace_dir / "target.c"

        # 备份原始文件，注入 Claude 版本
        original = target_src.read_text(errors="replace")
        target_src.write_text(edited_file.read_text(errors="replace"))

        # 构建 west build 命令
        cmd = task["run_command"]
        test_dir = ""
        if "&&" in cmd:
            parts = cmd.split("&&")
            first = parts[0].strip()
            if first.startswith("cd "):
                test_dir = first[3:].strip()
            parts = [p.strip() for p in parts if not p.strip().startswith("cd ")]
            cmd = " && ".join(parts)

        cwd = str(ZEPHYR_BASE / test_dir) if test_dir else str(ZEPHYR_BASE)
        env = os.environ.copy()
        env["ZEPHYR_BASE"] = str(ZEPHYR_BASE)
        env["ZEPHYR_SDK_INSTALL_DIR"] = str(ZEPHYR_SDK_DIR)

        try:
            print(f"  [VERIFY] {cwd}")
            p = subprocess.run(cmd, shell=True, capture_output=True, text=True,
                               timeout=TEST_TIMEOUT, cwd=cwd, env=env)
        except subprocess.TimeoutExpired:
            target_src.write_text(original)  # 恢复
            return False, "[TIMEOUT]", -1, ERROR_TIMEOUT

        stdout = p.stdout or ""
        stderr = p.stderr or ""
        test_out = stdout + "\n--- stderr ---\n" + stderr

        # 恢复原始文件
        target_src.write_text(original)

        # 解析 ztest 输出
        build_fail = bool(re.search(r"(?:BUILD FAILED|FATAL ERROR|ninja: error"
            r"|undefined reference)", stdout + stderr))
        fatal_fail = bool(re.search(r"(?:Segmentation fault|Aborted|panic)", stdout + stderr))

        target_test = task.get("unit_test", "")
        specific_fail = bool(target_test and re.search(
            rf"FAIL\s+-\s+{re.escape(target_test)}", stdout))

        total_failures = len(re.findall(r'FAIL\s+-\s+', stdout))

        project_success = "PROJECT EXECUTION SUCCESSFUL" in stdout
        project_failed = "PROJECT EXECUTION FAILED" in stdout

        passed = (not build_fail and not fatal_fail and not specific_fail
                  and project_success)

        error_cat = None
        if build_fail:
            passed = False
            error_cat = ERROR_COMPILE
        elif fatal_fail:
            passed = False
            error_cat = ERROR_CRASH
        elif specific_fail:
            passed = False
            error_cat = ERROR_TEST_FAIL
        elif project_failed:
            passed = False
            error_cat = ERROR_TEST_FAIL
        elif not project_success:
            passed = False
            error_cat = ERROR_TEST_NOT_RUN

        return passed, test_out, total_failures, error_cat

    # ═══════════════════════════════════════════════════════════════
    # 日志 / 结果
    # ═══════════════════════════════════════════════════════════════

    def _record_trajectory(self, task, claude_out, diff, illegal):
        traj_file = TRAJECTORY_DIR / f"task_{task['task_id']}.log"
        parts = []
        if claude_out:
            t = claude_out
            if len(t) > MAX_OUTPUT_LENGTH:
                half = MAX_OUTPUT_LENGTH // 2
                t = t[:half] + f"\n[... TRUNCATED {len(t)-2*half} bytes ...]\n" + t[-half:]
            parts.append(f"{'='*60}\n=== Claude Session ===\n{'='*60}\n{t}\n")
        if diff and diff.strip():
            parts.append(f"{'='*60}\n=== Diff ===\n{'='*60}\n{diff}\n")
        if illegal:
            parts.append(f"{'='*60}\n=== ILLEGAL ===\n{'='*60}\n" + "\n".join(illegal) + "\n")
        append_trajectory(traj_file, "\n".join(parts))

    def _make_result(self, task, start, passed, claude_completed, illegal,
                     error=None, error_category=None, diff=None,
                     file_tampering=None, snapshot=None,
                     cleanup_errors=None, total_test_failures=None):
        result = {
            "task_id": task["task_id"], "passed": passed,
            "claude_completed": claude_completed,
            "runtime_s": round(time.time() - start, 1),
            "illegal_changes": illegal,
            "file_tampering": file_tampering or [],
            "error": error, "error_category": error_category or ERROR_NONE,
        }
        if diff is not None:
            result["patch"] = diff
        if snapshot is not None:
            result["snapshot"] = snapshot
        if cleanup_errors:
            result["cleanup_errors"] = cleanup_errors
        if total_test_failures is not None:
            result["total_test_failures"] = total_test_failures
        return result
