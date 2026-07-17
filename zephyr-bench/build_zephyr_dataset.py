#!/usr/bin/env python3
"""
Build Zephyr code-completion benchmark dataset using tree-sitter AST parsing.

Flow:
1. Read zephyr_tests.jsonl -> get test metadata (function, file, suite, dir, run_command)
2. Use tree-sitter to parse test files -> extract all called function names (SUT candidates)
3. Search Zephyr source directories for these function definitions
4. Select best definition by directory priority (lib/ > subsys/ > arch/ > drivers/)
5. Generate masked_code + oracle for each function -> output JSONL task files

Usage:
    python build_zephyr_dataset.py --repo ../zephyr --tests zephyr_tests.jsonl --out-prefix zephyr_tasks
"""

import argparse
import json
import re
import sys
from pathlib import Path

from tree_sitter import Parser, Language
import tree_sitter_c
from mask_engine import mask_function

C_LANG = Language(tree_sitter_c.language())
parser = Parser(C_LANG)

# Source directory priority: higher number = more likely to contain the real SUT
SOURCE_PRIORITY = {
    "lib":     5,
    "subsys":  4,
    "arch":    3,
    "drivers": 2,
    "kernel":  3,
}
DEFAULT_PRIORITY = 1

# Top-level directories to skip entirely during source file scanning
EXCLUDED_DIRS = {
    "tests", "build", "doc", "scripts", "boards", "soc",
    "dts", ".github", "cmake", "snippets", "share",
}

# Framework / stdlib / macro names to exclude from SUT candidates
FRAMEWORK_FUNCS = {
    # Zephyr zassert / zassume / zexpect macros
    "zassert_true", "zassert_false", "zassert_ok", "zassert_equal",
    "zassert_equal_ptr", "zassert_not_equal", "zassert_within",
    "zassert_mem_equal", "zassert_str_equal",
    "zassume_true", "zassume_false", "zassume_ok", "zassume_equal",
    "zexpect_true", "zexpect_false", "zexpect_ok", "zexpect_equal",
    # Zephyr test infrastructure
    "ztest_test_suite", "ztest_unit_test", "ztest_register_test_suite",
    "ztest_run_test_suites", "ztest_run_test_suite",
    "ZTEST", "ZTEST_F",
    # Zephyr common macros
    "printk", "sys_putchar", "z_log_msg", "z_log_msg2",
    "log_output_flush", "log_output_msg_process",
    "IS_ENABLED", "ARG_UNUSED", "COND_CODE_1", "UTIL_CAT",
    "__ASSERT", "__ASSERT_NO_MSG", "__ASSERT_LOCAL",
    "BUILD_ASSERT", "BUILD_ASSERT_MSG",
    "k_sleep", "k_yield", "k_ms_to_ticks_ceil64",
    "k_cycle_get_32", "k_cycle_get_64",
    "sys_clock_cycles_per_sec",
    # C standard library
    "strlen", "sizeof", "memcmp", "memcpy", "memset", "printf",
    "calloc", "malloc", "free", "abort", "exit", "strcmp", "strcpy",
    "sprintf", "snprintf", "fprintf", "puts", "putchar", "getchar",
    "atoi", "atol", "atof", "strtol", "strtoul", "strtod",
    "memchr", "memmove", "strcat", "strncat", "strncpy",
    "strchr", "strrchr", "strstr", "strtok", "strerror", "perror",
    "qsort", "bsearch", "rand", "srand", "time", "clock",
    "tolower", "toupper", "isalpha", "isdigit", "isalnum",
    "isspace", "islower", "isupper", "isprint", "iscntrl",
    "assert", "abs", "labs", "llabs", "min", "max",
    # Additional common Zephyr functions
    "strlcpy", "strlcat",
}

C_KEYWORDS = {
    "auto", "break", "case", "char", "const", "continue", "default", "do",
    "double", "else", "enum", "extern", "float", "for", "goto", "if",
    "int", "long", "register", "return", "short", "signed", "sizeof",
    "static", "struct", "switch", "typedef", "union", "unsigned", "void",
    "volatile", "while", "inline", "restrict", "_Bool", "_Complex", "_Imaginary",
}

# Words to skip when extracting function names from signatures
_SIG_SKIP = {"if", "for", "while", "switch", "return", "__attribute__", "__attribute"}


def is_excluded(rel_path: str) -> bool:
    top = rel_path.split("/", 1)[0] if "/" in rel_path else rel_path
    return top in EXCLUDED_DIRS


def path_priority(rel_path: str) -> int:
    top = rel_path.split("/", 1)[0] if "/" in rel_path else rel_path
    return SOURCE_PRIORITY.get(top, DEFAULT_PRIORITY)


def clean_identifier(raw: str) -> str:
    while raw and not (raw[-1].isalnum() or raw[-1] == "_"):
        raw = raw[:-1]
    return raw


def is_valid_func_name(name: str) -> bool:
    if not name or not name[0].isalpha() or name.startswith("_"):
        return False
    if name.isupper() and len(name) > 1:
        return False
    return all(c.isalnum() or c == "_" for c in name)


def extract_function_body(node, code: str) -> str:
    start = node.start_byte
    end = node.end_byte
    while start > 0 and (code[start - 1].isalnum() or code[start - 1] == "_"):
        start -= 1
    return code[start:end]


def get_func_name(node, code: str):
    body = extract_function_body(node, code)
    sig = body.split("{")[0] if "{" in body else body
    sig = re.sub(r"__attribute__\s*\(\([^)]*\)\)", "", sig)
    for m in re.finditer(r"(\w+)\s*\(", sig):
        name = clean_identifier(m.group(1))
        if name and name not in _SIG_SKIP:
            return name
    return None


def extract_functions(code: str):
    """Extract all function definitions from source code using tree-sitter."""
    tree = parser.parse(bytes(code, "utf8"))
    results = []
    seen = set()

    stack = [tree.root_node]
    while stack:
        node = stack.pop()
        if node.type == "function_definition":
            name = get_func_name(node, code)
            body = extract_function_body(node, code)
            if name and is_valid_func_name(name):
                key = (name, node.start_byte)
                if key not in seen:
                    seen.add(key)
                    results.append({
                        "name": name,
                        "start": node.start_byte,
                        "end": node.end_byte,
                        "body": body,
                    })
        stack.extend(node.children)

    return results


def extract_called_functions(code: str):
    """Extract all called function names from test file AST."""
    tree = parser.parse(bytes(code, "utf8"))
    called = set()

    stack = [tree.root_node]
    while stack:
        node = stack.pop()
        if node.type == "call_expression":
            func_node = node.child_by_field_name("function")
            if func_node and func_node.type == "identifier":
                start = func_node.start_byte
                end = func_node.end_byte
                while start > 0 and (code[start - 1].isalnum()
                                     or code[start - 1] == "_"):
                    start -= 1
                name = clean_identifier(code[start:end])
                if (is_valid_func_name(name)
                        and name not in FRAMEWORK_FUNCS
                        and not name.startswith("test_")
                        and not name.startswith("zassert_")
                        and not name.startswith("zassume_")
                        and not name.startswith("zexpect_")):
                    called.add(name)
        stack.extend(node.children)

    return called


def find_sut_functions(repo_root, tests):
    """Extract SUT function names from test files."""
    repo_root = Path(repo_root)
    test_to_sut = {}
    all_sut = set()

    for test in tests:
        test_func = test.get("test_function")
        test_file = test.get("test_file")
        if not test_func or not test_file:
            continue

        path = repo_root / test_file
        if not path.exists():
            continue

        try:
            code = path.read_text(errors="replace")
            funcs = extract_called_functions(code)
            funcs = {f for f in funcs if f and f[0].isalpha() and not f.startswith("_")}
            if funcs:
                test_to_sut[test_func] = funcs
                all_sut.update(funcs)
        except Exception:
            continue

    return test_to_sut, all_sut


def find_function_definitions(repo_root, func_names):
    """Search all source files for function definitions, picking best by priority."""
    repo_root = Path(repo_root)
    best = {}

    for ext in ("*.c", "*.h"):
        for file in repo_root.rglob(ext):
            rel = file.relative_to(repo_root).as_posix()
            if is_excluded(rel):
                continue

            try:
                text = file.read_text(errors="replace")
            except Exception:
                continue

            for func in extract_functions(text):
                name = func["name"]
                if name not in func_names:
                    continue

                prio = path_priority(rel)
                prev_prio, _ = best.get(name, (0, None))
                if prio > prev_prio:
                    best[name] = (prio, {
                        "file": rel,
                        "body": func["body"],
                        "start": func["start"],
                    })

    return {name: info for name, (_, info) in best.items()}


def select_best_test(func_name, test_to_sut, test_commands, test_info_map):
    """Select the most relevant test function and run command for a SUT function."""
    relevant = []
    for test_func, sut_funcs in test_to_sut.items():
        if func_name in sut_funcs:
            score = 2 if func_name in test_func else 1
            relevant.append((score, test_func))

    if not relevant:
        return None, None, None

    relevant.sort(key=lambda x: -x[0])
    best_func = relevant[0][1]
    info = test_info_map.get(best_func, {})
    return best_func, test_commands.get(best_func), info


def build(repo_root, tests_path, out_prefix):
    repo_root = Path(repo_root).resolve()
    out_dir = Path(out_prefix).parent.resolve() or Path.cwd()

    # Load test metadata
    tests = []
    test_commands = {}
    test_info_map = {}
    with open(tests_path, "r", encoding="utf-8") as f:
        for line in f:
            line = line.strip()
            if not line:
                continue
            try:
                t = json.loads(line)
                tests.append(t)
                tf = t.get("test_function")
                cmd = t.get("run_command")
                if tf and cmd:
                    test_commands[tf] = cmd
                if tf:
                    test_info_map[tf] = {
                        "suite": t.get("suite", ""),
                        "zephyr_test_module": t.get("test_base", ""),
                    }
            except json.JSONDecodeError:
                continue

    print(f"Loaded {len(tests)} tests, {len(test_commands)} with commands")

    # Extract SUT function names
    test_to_sut, sut_funcs = find_sut_functions(repo_root, tests)
    print(f"Found {len(sut_funcs)} unique SUT function names from test files")

    # Find definitions by priority
    func_locations = find_function_definitions(repo_root, sut_funcs)
    print(f"Found definitions for {len(func_locations)} functions")

    # Report missing
    missing = sut_funcs - set(func_locations.keys())
    if missing:
        print(f"Missing definitions for {len(missing)} functions:")
        for fn in sorted(missing)[:20]:
            print(f"  - {fn}")
        if len(missing) > 20:
            print(f"  ... and {len(missing) - 20} more")

    # Prepare output
    oracles_dir = out_dir / "oracles"
    oracles_dir.mkdir(parents=True, exist_ok=True)

    c_out = Path(out_prefix + ".c.jsonl")
    h_out = Path(out_prefix + ".h.jsonl")

    def make_task(func_name, func_info):
        masked, target = mask_function(func_info["body"])
        if masked is None:
            return None
        unit_test, run_command, extra_info = select_best_test(
            func_name, test_to_sut, test_commands, test_info_map
        )
        return {
            "source_path": func_info["file"],
            "sut_function": func_name,
            "masked_code": masked,
            "unit_test": unit_test or "",
            "run_command": run_command or "",
            "func_body": func_info["body"],
            "suite": extra_info.get("suite", "") if extra_info else "",
            "zephyr_test_module": extra_info.get("zephyr_test_module", "") if extra_info else "",
        }

    c_tasks = []
    h_tasks = []
    for func_name, func_info in sorted(func_locations.items(), key=lambda x: x[0]):
        t = make_task(func_name, func_info)
        if t is None:
            continue
        if func_info["file"].endswith(".h"):
            h_tasks.append(t)
        else:
            c_tasks.append(t)

    def write_tasks(tasks, suffix, path):
        with open(path, "w", encoding="utf-8") as f:
            for tid, task in enumerate(tasks, 1):
                oracle_name = f"{tid}.{suffix}"
                oracle_path = oracles_dir / oracle_name
                oracle_path.write_text(task["func_body"], encoding="utf-8")

                record = {
                    "task_id": str(tid),
                    "source_path": task["source_path"],
                    "sut_function": task["sut_function"],
                    "oracle": oracle_path.relative_to(out_dir).as_posix(),
                    "masked_code": task["masked_code"],
                    "unit_test": task["unit_test"],
                    "run_command": task["run_command"],
                    "suite": task["suite"],
                    "zephyr_test_module": task["zephyr_test_module"],
                }
                f.write(json.dumps(record, ensure_ascii=False) + "\n")

    write_tasks(c_tasks, "c", c_out)
    write_tasks(h_tasks, "h", h_out)
    print(f"Generated {len(c_tasks)} C-file tasks -> {c_out}")
    print(f"Generated {len(h_tasks)} header tasks -> {h_out}")


def main():
    parser = argparse.ArgumentParser(
        description="Build Zephyr code completion benchmark dataset"
    )
    parser.add_argument("--repo", required=True, help="Zephyr repo root")
    parser.add_argument("--tests", default="zephyr_tests.jsonl",
                        help="Path to zephyr_tests.jsonl")
    parser.add_argument("--out-prefix", required=True,
                        help="Output prefix (e.g. zephyr_tasks -> zephyr_tasks.c.jsonl + zephyr_tasks.h.jsonl)")
    args = parser.parse_args()

    build(args.repo, args.tests, args.out_prefix)


if __name__ == "__main__":
    main()
