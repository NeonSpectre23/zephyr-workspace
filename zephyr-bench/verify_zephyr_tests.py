#!/usr/bin/env python3
"""
Verify Zephyr test commands from zephyr_tasks.jsonl or zephyr_tests.jsonl.

Two modes:
1. west build mode (default): run `west build -b native_sim -p` + run zephyr.elf
2. lightweight mode (--lightweight): GCC compile with mocked ztest headers

Parses ztest output format:
  PASS - test_name
  FAIL - test_name
  PROJECT EXECUTION SUCCESSFUL

Output: verify_results.json + verify_logs/
"""

import argparse
import hashlib
import json
import os
import re
import subprocess
import sys
import tempfile
from pathlib import Path
from collections import defaultdict

# ──────────────────────────── ZTEST mock header ────────────────────────────

ZTEST_MOCK = r"""
#ifndef ZEPHYR_INCLUDE_ZTEST_H_
#define ZEPHYR_INCLUDE_ZTEST_H_
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

/* Error codes */
#define EINVAL 22
#define ENOMEM 12
#define ENOSPC 28
#define ERANGE 34
#define ENOTSUP 134
#define EIO    5
#define EAGAIN 11
#define ENODATA 61
#define EBADMSG 74

/* Compiler attributes */
#define __weak          __attribute__((__weak__))
#define __printf_like(f, a) __attribute__((format(printf, f, a)))
#define __must_check    __attribute__((warn_unused_result))
#define __deprecated
#define __maybe_unused  __attribute__((unused))
#define __aligned(x)    __attribute__((__aligned__(x)))
#define __packed        __attribute__((__packed__))
#define __used          __attribute__((__used__))

/* Ztest macros */
#define ZTEST(suite, fn) static void fn(void)
#define ZTEST_F(suite, fn) static void fn(void)
#define ZTEST_USER(suite, fn) static void fn(void)
#define ZTEST_USER_F(suite, fn) static void fn(void)
#define ZTEST_SUITE(suite, ...)
#define ZTEST_RULE(rule, fn)
#define ztest_test_suite(suite, ...)
#define ztest_unit_test(test)
#define ztest_register_test_suite(suite)

/* Assert macros */
#define zassert_true(cond, ...)          do { if (!(cond)) return; } while(0)
#define zassert_false(cond, ...)         do { if (cond) return; } while(0)
#define zassert_ok(cond, ...)            do { if (!(cond)) return; } while(0)
#define zassert_equal(a, b, ...)         do { if ((a)!=(b)) return; } while(0)
#define zassert_not_equal(a, b, ...)     do { if ((a)==(b)) return; } while(0)
#define zassert_equal_ptr(a, b, ...)     do { if ((a)!=(b)) return; } while(0)
#define zassert_within(a, b, c, ...)
#define zassert_mem_equal(a, b, c, ...)
#define zassert_str_equal(a, b, ...)
#define zassume_true(cond, ...)          do { if (!(cond)) return; } while(0)
#define zassume_false(cond, ...)         do { if (cond) return; } while(0)
#define zassume_equal(a, b, ...)         do { if ((a)!=(b)) return; } while(0)
#define zexpect_true(cond, ...)          do { if (!(cond)) return; } while(0)
#define zexpect_equal(a, b, ...)         do { if ((a)!=(b)) return; } while(0)

/* Minimal Zephyr types */
typedef uint8_t  u8_t;
typedef uint16_t u16_t;
typedef uint32_t u32_t;
typedef uint64_t u64_t;
typedef int8_t   s8_t;
typedef int16_t  s16_t;
typedef int32_t  s32_t;
typedef int64_t  s64_t;

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))
#define UNUSED(x) ((void)(x))
#define ARG_UNUSED(x) UNUSED(x)
#define IS_ENABLED(x) 0
#define CONTAINER_OF(ptr, type, field) ((type *)(((char *)(ptr)) - offsetof(type, field)))
#define BIT(n) (1UL << (n))

#define __in_section(_name, _subname)

/* Minimal logging */
#define printk(fmt, ...) printf(fmt, ##__VA_ARGS__)
#define LOG_MODULE_REGISTER(...)
#define LOG_ERR(fmt, ...) fprintf(stderr, fmt "\n", ##__VA_ARGS__)
#define LOG_WRN(fmt, ...) fprintf(stderr, fmt "\n", ##__VA_ARGS__)
#define LOG_INF(fmt, ...) printf(fmt "\n", ##__VA_ARGS__)
#define LOG_DBG(fmt, ...) printf(fmt "\n", ##__VA_ARGS__)

/* TC_END_REPORT */
#define TC_END_REPORT(result) ((void)(result))
#define TC_END(result) ((void)(result))

#endif /* ZEPHYR_INCLUDE_ZTEST_H_ */
"""

# ────────────────── Lightweight GCC verification ──────────────────

def generate_forward_decls(source_code: str) -> str:
    """Generate forward declarations for all function definitions found in source."""
    decls = []
    # Match function definitions: return_type func_name(params) {
    pattern = re.compile(
        r'^([a-zA-Z_][\w\s*]+?)\s+(\w+)\s*\(([^)]*)\)\s*\{',
        re.MULTILINE
    )
    for m in pattern.finditer(source_code):
        ret_type = m.group(1).strip()
        func_name = m.group(2)
        params = m.group(3)
        # Skip keywords and macros
        if ret_type in ('if', 'for', 'while', 'switch', 'return', 'sizeof'):
            continue
        if func_name.startswith('_'):
            continue
        # Simplify return type
        ret_type = ret_type.replace('\n', ' ').replace('\t', ' ')
        ret_type = ' '.join(ret_type.split())
        decls.append(f"{ret_type} {func_name}({params});")
    return '\n'.join(decls)


def lightweight_verify(test_entry: dict, repo_root: Path, logs_dir: Path) -> dict:
    """Verify a single task by compiling the oracle function body with GCC."""
    test_func = test_entry.get("unit_test") or test_entry.get("test_function", "")
    test_base = test_entry.get("test_base", test_entry.get("zephyr_test_module", "unknown"))
    func_body = test_entry.get("func_body", "")
    source_path = test_entry.get("source_path", "")

    if not func_body:
        # If no func_body in entry, try reading from oracle
        oracle_path = test_entry.get("oracle", "")
        if oracle_path:
            try:
                func_body = Path(oracle_path).read_text(errors="replace")
            except Exception:
                return {"status": "NO_ORACLE", "module": test_base, "trace": "cannot read oracle"}

    if not func_body:
        return {"status": "NO_BODY", "module": test_base, "trace": "no function body available"}

    # Build compile unit: mock headers + forward decls + function body
    parts = [ZTEST_MOCK]

    # Generate forward declarations for any function calls within the body
    fwd = generate_forward_decls(func_body)
    if fwd:
        parts.append(fwd)

    parts.append(func_body)

    # If the function uses undeclared types, add common struct stubs
    if "struct tm" in func_body:
        parts.insert(1, "struct tm { int tm_sec, tm_min, tm_hour, tm_mday, tm_mon, tm_year, tm_wday, tm_yday, tm_isdst; };")

    compile_unit = '\n'.join(parts)

    # Write to temp file
    safe_name = test_func.replace('/', '_').replace('\\', '_')[:60]
    tmp_path = logs_dir / f"verify_{safe_name}.c"
    tmp_path.write_text(compile_unit, encoding="utf-8")

    # Find working GCC
    gcc_cmd = None
    use_wsl = False

    try:
        r = subprocess.run(["gcc", "--version"], capture_output=True, timeout=5)
        if r.returncode == 0:
            with tempfile.NamedTemporaryFile(suffix=".c", mode="w", delete=False) as tf:
                tf.write("int main(){return 0;}")
                tf_path = tf.name
            r2 = subprocess.run(
                ["gcc", "-c", "-x", "c", tf_path, "-o", "/dev/null"],
                capture_output=True, timeout=10
            )
            os.unlink(tf_path)
            if r2.returncode == 0:
                gcc_cmd = ["gcc"]
    except Exception:
        pass

    if gcc_cmd is None:
        try:
            r = subprocess.run(
                ["wsl", "gcc", "--version"], capture_output=True, timeout=10,
                env={**os.environ, "MSYS2_ARG_CONV_EXCL": "*"}
            )
            if r.returncode == 0:
                gcc_cmd = ["wsl", "gcc"]
                use_wsl = True
        except Exception:
            pass

    if gcc_cmd is None:
        return {"status": "NO_GCC", "module": test_base, "trace": "no working gcc found"}

    if use_wsl:
        gcc_input = str(tmp_path).replace("\\", "/").replace("D:", "/mnt/d").replace("d:", "/mnt/d")
    else:
        gcc_input = str(tmp_path)

    try:
        env = {**os.environ, "MSYS2_ARG_CONV_EXCL": "*"} if use_wsl else None
        proc = subprocess.run(
            gcc_cmd + ["-c", "-w", "-x", "c", gcc_input, "-o", "/dev/null"],
            capture_output=True, text=True, timeout=30, env=env,
            encoding="utf-8", errors="replace"
        )
    except subprocess.TimeoutExpired:
        return {"status": "TIMEOUT", "module": test_base, "trace": "gcc timeout"}
    except FileNotFoundError:
        return {"status": "NO_GCC", "module": test_base, "trace": "gcc not found"}

    if proc.returncode == 0:
        return {"status": "COMPILE_PASS", "module": test_base, "trace": ""}
    else:
        combined = (proc.stderr or "") + (proc.stdout or "")
        errors = [l for l in combined.split('\n') if 'error:' in l]
        trace = '; '.join(errors[:3]) if errors else combined[:300]
        return {"status": "COMPILE_FAIL", "module": test_base, "trace": trace}

    if proc.returncode == 0:
        return {"status": "COMPILE_PASS", "module": test_base, "trace": ""}
    else:
        # Extract first few errors
        errors = [l for l in proc.stderr.split('\n') if 'error:' in l]
        trace = '; '.join(errors[:3]) if errors else proc.stderr[:200]
        return {"status": "COMPILE_FAIL", "module": test_base, "trace": trace}


# ────────────────── West build verification ──────────────────

def make_log_name(cmd: str, module: str) -> str:
    h = hashlib.md5(cmd.encode()).hexdigest()[:8]
    safe = module.replace("/", "_").replace("\\", "_")
    return f"{safe}_{h}.log"


def verify_west_build(tests: list, repo_root: Path, out_dir: Path, timeout: int):
    """Run west build verification for test entries grouped by run_command."""
    logs_dir = out_dir / "verify_logs"
    logs_dir.mkdir(parents=True, exist_ok=True)

    # Group by run_command
    commands = defaultdict(list)
    for t in tests:
        cmd = t.get("run_command", "")
        if cmd:
            commands[cmd].append(t)

    unique_commands = sorted(commands.keys())
    print(f"Total tests: {len(tests)}, unique commands: {len(unique_commands)}")

    test_results = {}

    for i, cmd in enumerate(unique_commands, 1):
        module_tests = commands[cmd]
        module_name = module_tests[0].get("test_base", "unknown")
        log_file = logs_dir / make_log_name(cmd, module_name)
        expected = len(module_tests)

        print(f"\n[{i}/{len(unique_commands)}] {module_name} ({expected} tests)")

        try:
            # Run from repo_root so that relative paths in run_command work
            proc = subprocess.run(
                cmd, shell=True, capture_output=True, text=True,
                timeout=timeout, cwd=str(repo_root)
            )
            stdout, stderr, rc = proc.stdout, proc.stderr, proc.returncode

            # Save log
            log_content = f"=== COMMAND ===\n{cmd}\n\n=== STDOUT ===\n{stdout}\n=== STDERR ===\n{stderr}\n=== RC ===\n{rc}\n"
            log_file.write_text(log_content, encoding="utf-8", errors="replace")

            if rc != 0:
                for t in module_tests:
                    tf = t.get("unit_test") or t.get("test_function", "unknown")
                    test_results[tf] = {
                        "status": "BUILD_FAIL", "module": t.get("test_base", ""),
                        "trace": f"rc={rc}, log={log_file.name}",
                    }
                print(f"  BUILD_FAIL (rc={rc})")
                continue

            # Parse ztest output: "PASS - test_name" / "FAIL - test_name"
            pass_count = len(re.findall(r'PASS\s+-\s+', stdout))
            fail_count = len(re.findall(r'FAIL\s+-\s+', stdout))

            if "PROJECT EXECUTION SUCCESSFUL" in stdout:
                for t in module_tests:
                    tf = t.get("unit_test") or t.get("test_function", "unknown")
                    test_results[tf] = {
                        "status": "PASS", "module": t.get("test_base", ""),
                        "trace": f"log={log_file.name}",
                    }
                print(f"  PASS ({pass_count}P/{fail_count}F)")
            elif pass_count > 0 or fail_count > 0:
                for t in module_tests:
                    tf = t.get("unit_test") or t.get("test_function", "unknown")
                    test_results[tf] = {
                        "status": "PARTIAL_PASS", "module": t.get("test_base", ""),
                        "trace": f"{pass_count}P/{fail_count}F, log={log_file.name}",
                    }
                print(f"  PARTIAL ({pass_count}P/{fail_count}F)")
            else:
                for t in module_tests:
                    tf = t.get("unit_test") or t.get("test_function", "unknown")
                    test_results[tf] = {
                        "status": "PARSE_ERROR", "module": t.get("test_base", ""),
                        "trace": f"no ztest output found, log={log_file.name}",
                    }
                print("  PARSE_ERROR")

        except subprocess.TimeoutExpired:
            print(f"  TIMEOUT ({timeout}s)")
            for t in module_tests:
                tf = t.get("unit_test") or t.get("test_function", "unknown")
                test_results[tf] = {
                    "status": "TIMEOUT", "module": t.get("test_base", ""),
                    "trace": f"timeout {timeout}s",
                }
        except Exception as e:
            print(f"  ERROR: {e}")
            for t in module_tests:
                tf = t.get("unit_test") or t.get("test_function", "unknown")
                test_results[tf] = {
                    "status": "ERROR", "module": t.get("test_base", ""),
                    "trace": str(e),
                }

    return test_results, logs_dir


# ────────────────── Main ──────────────────

def main():
    parser = argparse.ArgumentParser(description="Verify Zephyr test commands")
    parser.add_argument("--input", default="zephyr_tests.jsonl",
                        help="Input JSONL file (zephyr_tests.jsonl or zephyr_tasks.*.jsonl)")
    parser.add_argument("--zephyr-root", default="../zephyr",
                        help="Zephyr repo root")
    parser.add_argument("--out-dir", default=".",
                        help="Output directory")
    parser.add_argument("--lightweight", action="store_true",
                        help="Use GCC lightweight verification instead of west build")
    parser.add_argument("--timeout", type=int, default=600,
                        help="Timeout per west build in seconds (default: 600)")
    parser.add_argument("--start", type=int, default=0,
                        help="Start at module index N (for incremental runs)")
    parser.add_argument("--limit", type=int, default=0,
                        help="Limit to N modules (0 = all)")
    args = parser.parse_args()

    repo_root = Path(args.zephyr_root).resolve()
    out_dir = Path(args.out_dir).resolve()
    input_file = Path(args.input)
    if not input_file.is_absolute():
        input_file = Path.cwd() / input_file

    if not input_file.exists():
        print(f"Error: {input_file} not found")
        sys.exit(1)

    # Load tests
    tests = []
    with open(input_file, "r", encoding="utf-8") as f:
        for line in f:
            line = line.strip()
            if not line:
                continue
            try:
                tests.append(json.loads(line))
            except json.JSONDecodeError:
                continue

    print(f"Loaded {len(tests)} entries from {input_file}")
    logs_dir = out_dir / "verify_logs"
    logs_dir.mkdir(parents=True, exist_ok=True)

    if args.lightweight:
        print("Mode: lightweight (GCC compile)")
        test_results = {}
        for i, t in enumerate(tests):
            if args.start > 0 and i < args.start:
                continue
            if args.limit > 0 and (i - args.start) >= args.limit:
                break

            test_func = t.get("unit_test") or t.get("test_function", f"entry_{i}")
            if (i + 1) % 50 == 0:
                print(f"  ... {i+1}/{len(tests)}")

            result = lightweight_verify(t, repo_root, logs_dir)
            result["test_function"] = test_func
            test_results[test_func] = result
    else:
        print("Mode: west build")
        # For west build, we need run_command in the entries
        has_commands = sum(1 for t in tests if t.get("run_command"))
        print(f"Entries with run_command: {has_commands}/{len(tests)}")

        if args.limit > 0:
            # Group by cmd and take first N modules
            cmds = defaultdict(list)
            for t in tests:
                cmd = t.get("run_command", "")
                if cmd:
                    cmds[cmd].append(t)
            unique = sorted(cmds.keys())
            selected = unique[args.start:args.start + args.limit]
            filtered = [t for cmd in selected for t in cmds[cmd]]
            print(f"Processing {len(selected)} modules ({len(filtered)} tests)")
            test_results, logs_dir = verify_west_build(
                filtered, repo_root, out_dir, args.timeout
            )
        else:
            test_results, logs_dir = verify_west_build(
                tests, repo_root, out_dir, args.timeout
            )

    # Summary
    statuses = defaultdict(int)
    for v in test_results.values():
        statuses[v["status"]] += 1

    print("\n" + "=" * 60)
    print(f"Results: {len(test_results)} total")
    for s in sorted(statuses.keys()):
        print(f"  {s:20s} {statuses[s]:>5d}")

    # Output
    out_file = out_dir / "verify_results.json"
    with open(out_file, "w", encoding="utf-8") as f:
        json.dump(test_results, f, indent=2, ensure_ascii=False)
    print(f"\nResults saved to {out_file}")
    print(f"Logs saved to {logs_dir}/")


if __name__ == "__main__":
    main()
