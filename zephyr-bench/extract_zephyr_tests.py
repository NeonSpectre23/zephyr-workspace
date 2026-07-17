#!/usr/bin/env python3
"""
Extract test metadata from Zephyr RTOS unit tests.

Scans tests/unit/, tests/subsys/, tests/lib/ for ZTEST / ZTEST_F blocks,
identifies candidate source files, and generates zephyr_tests.jsonl.

Usage:
    python extract_zephyr_tests.py --zephyr-root ./zephyr [--categories unit,lib,subsys]
"""

import argparse
import hashlib
import json
import re
import sys
from pathlib import Path
from collections import Counter

# Regex to match ZTEST(suite, name) or ZTEST_F(suite, name) followed by {
ZTEST_PATTERN = re.compile(
    r'ZTEST(?:_F)?\s*\(\s*(\w+)\s*,\s*(test_\w+(?:_\w+)*)\s*\)\s*\{',
    re.DOTALL
)

# Regex to extract #include "*.c" paths from test files
INCLUDE_C_PATTERN = re.compile(
    r'^\s*#include\s+"((?:[^"]|\\")+\.c)"\s*$',
    re.MULTILINE
)

# Regex to extract target_sources() entries from CMakeLists.txt
TARGET_SOURCES_PATTERN = re.compile(
    r'target_sources\s*\([^)]+PRIVATE\s+([^)]+)\)',
    re.DOTALL
)
FILE_GLOB_PATTERN = re.compile(
    r'FILE\s*\(\s*GLOB\s+\w+\s+([^)]+)\)',
    re.DOTALL
)

CATEGORY_ROOTS = {
    "unit":   "tests/unit",
    "lib":    "tests/lib",
    "subsys": "tests/subsys",
}


def normalise(name: str) -> str:
    """Strip test_ prefix and __suffix from function names."""
    if name.startswith("test_"):
        name = name[5:]
    return name.split("__")[0]


def find_ztest_blocks(file_path: Path):
    """Extract all ZTEST/ZTEST_F blocks from a C file."""
    try:
        content = file_path.read_text(errors="replace")
    except Exception:
        return []

    results = []
    for m in ZTEST_PATTERN.finditer(content):
        suite = m.group(1)
        test_name = m.group(2)
        line_num = content[:m.start()].count('\n') + 1
        results.append({
            "suite": suite,
            "test_name": test_name,
            "start_line": line_num,
            "file": str(file_path),
        })
    return results


def extract_include_sources(file_path: Path):
    """Extract #include'd .c source paths from a test file."""
    try:
        content = file_path.read_text(errors="replace")
    except Exception:
        return []

    results = []
    for m in INCLUDE_C_PATTERN.finditer(content):
        raw = m.group(1).replace('\\"', '"')
        resolved = (file_path.parent / raw).resolve()
        results.append(str(resolved))
    return results


def extract_cmake_sources(cmake_path: Path, repo_root: Path):
    """Extract source file references from CMakeLists.txt."""
    if not cmake_path.is_file():
        return []

    try:
        content = cmake_path.read_text(errors="replace")
    except Exception:
        return []

    results = []
    # Find target_sources(app PRIVATE ...) or target_sources(testbinary PRIVATE ...)
    for m in TARGET_SOURCES_PATTERN.finditer(content):
        sources_text = m.group(1)
        for src in re.findall(r'\S+', sources_text):
            # Skip CMake variable references
            if src.startswith('${'):
                continue
            resolved = (cmake_path.parent / src).resolve()
            results.append(str(resolved))

    # Also check FILE(GLOB ...)
    for m in FILE_GLOB_PATTERN.finditer(content):
        glob_text = m.group(1)
        for src in re.findall(r'\S+', glob_text):
            if src.startswith('${'):
                continue
            try:
                for matched in cmake_path.parent.glob(src):
                    results.append(str(matched.resolve()))
            except Exception:
                pass

    return results


def resolve_zephyr_base(path_str: str, repo_root: Path) -> str:
    """Resolve ${ZEPHYR_BASE} references in paths."""
    return path_str.replace('${ZEPHYR_BASE}', str(repo_root))


def find_candidate_sources(test_dir: Path, test_files, repo_root: Path):
    """Find candidate source files for a test module.

    Two strategies:
    1. Parse #include "*.c" lines from test source files (tests/unit/ pattern)
    2. Parse CMakeLists.txt for target_sources entries
    """
    candidates = set()

    # Strategy 1: #include "*.c" in test files
    for tf in test_files:
        for src in extract_include_sources(tf):
            candidates.add(src)

    # Strategy 2: CMakeLists.txt
    cmake = test_dir / "CMakeLists.txt"
    if cmake.is_file():
        for src in extract_cmake_sources(cmake, repo_root):
            resolved = resolve_zephyr_base(src, repo_root)
            candidates.add(resolved)

    return sorted(candidates)


def find_test_category(test_file: Path, repo_root: Path) -> str:
    """Determine which test category a file belongs to."""
    try:
        rel = test_file.relative_to(repo_root).as_posix()
    except ValueError:
        return "unknown"
    for cat, root in CATEGORY_ROOTS.items():
        if rel.startswith(root + "/"):
            return cat
    return "unknown"


def make_run_command(test_dir_rel: str) -> str:
    """Generate a west build run command for a test module.

    test_dir_rel is relative to repo root (e.g., 'tests/unit/base64').

    Unit tests use 'unit_testing' board (Zephyr 4.x unittest module requires it).
    All other tests use native_sim/native/64 to avoid gcc-multilib requirement.
    -DM64_MODE=1 used for unit tests since they default to -m32.
    """
    is_unit_test = test_dir_rel.startswith("tests/unit/")
    if is_unit_test:
        return (
            f"cd {test_dir_rel} && "
            f"west build -b unit_testing -p -d build "
            f"-- -DZEPHYR_TOOLCHAIN_VARIANT=host -DM64_MODE=1 && "
            f"./build/testbinary"
        )
    else:
        return (
            f"cd {test_dir_rel} && "
            f"west build -b native_sim/native/64 -p -d build "
            f"-- -DZEPHYR_TOOLCHAIN_VARIANT=host && "
            f"./build/zephyr/zephyr.exe"
        )


def scan_tests(repo_root: Path, categories):
    """Scan all requested test categories and return test entries."""
    all_entries = []

    for cat in categories:
        cat_root = repo_root / CATEGORY_ROOTS[cat]
        if not cat_root.is_dir():
            print(f"Warning: {cat_root} not found, skipping category '{cat}'")
            continue

        print(f"\nScanning {CATEGORY_ROOTS[cat]}/ ...")

        # Walk the test tree. Each test module is a directory containing
        # one or more .c files. The module root is where CMakeLists.txt lives.
        # We group .c files by their parent directory (the module root).

        module_dirs = {}  # module_root -> list of .c file paths
        for c_file in cat_root.rglob("*.c"):
            # Determine module root: the directory containing CMakeLists.txt
            # or the immediate parent if no CMakeLists.txt (some modules
            # like tests/unit/ have CMakeLists.txt at the module level)
            module_root = c_file.parent
            # Walk up until we find the module root (has CMakeLists.txt or
            # is the top-level test category directory)
            for parent in c_file.parents:
                if parent == cat_root or parent == repo_root:
                    break
                if (parent / "CMakeLists.txt").is_file():
                    module_root = parent
                    break

            if module_root not in module_dirs:
                module_dirs[module_root] = []
            module_dirs[module_root].append(c_file)

        print(f"  Found {len(module_dirs)} test module(s), {sum(len(v) for v in module_dirs.values())} .c file(s)")

        module_count = 0
        for module_root, c_files in sorted(module_dirs.items()):
            # Extract all ZTEST blocks from all .c files in this module
            all_blocks = []
            for cf in c_files:
                blocks = find_ztest_blocks(cf)
                all_blocks.extend(blocks)

            if not all_blocks:
                continue

            module_count += 1
            module_rel = str(module_root.relative_to(repo_root)).replace('\\', '/')
            test_base = module_root.name

            # Find candidate source files
            candidate_files = find_candidate_sources(module_root, c_files, repo_root)

            # Filter to files actually under repo_root
            candidate_files = [
                f for f in candidate_files
                if f.startswith(str(repo_root)) and "/tests/" not in f
            ]
            # Convert to relative paths
            candidate_files_rel = []
            for f in candidate_files:
                try:
                    candidate_files_rel.append(
                        Path(f).relative_to(repo_root).as_posix()
                    )
                except ValueError:
                    candidate_files_rel.append(f)

            run_cmd = make_run_command(module_rel)

            for block in all_blocks:
                test_func = block["test_name"]
                # Generate a stable ID
                test_id = hashlib.md5(
                    f"{test_func}:{module_rel}:{block['file']}".encode()
                ).hexdigest()[:8]

                file_rel = str(Path(block['file']).relative_to(repo_root)).replace('\\', '/')

                all_entries.append({
                    "test_id": test_id,
                    "test_function": test_func,
                    "normalized_function": normalise(test_func),
                    "test_file": file_rel,
                    "test_base": test_base,
                    "test_dir": module_rel,
                    "test_category": cat,
                    "suite": block["suite"],
                    "has_fixture": False,  # ZTEST_F detection can be added later
                    "candidate_files": candidate_files_rel,
                    "num_candidate_files": len(candidate_files_rel),
                    "suite_filterable": True,
                    "run_command": run_cmd,
                })

        print(f"  Extracted entries from {module_count} modules")

    return all_entries


def main():
    parser = argparse.ArgumentParser(
        description="Extract Zephyr test metadata for code-completion benchmark"
    )
    parser.add_argument("--zephyr-root", required=True, help="Zephyr repo root")
    parser.add_argument("--output", default="zephyr_tests.jsonl", help="Output JSONL path")
    parser.add_argument(
        "--categories", default="unit,lib,subsys",
        help="Comma-separated test categories to scan (default: unit,lib,subsys)"
    )
    parser.add_argument("--max-modules", type=int, default=0,
                        help="Limit to first N modules per category (0 = all)")
    args = parser.parse_args()

    repo_root = Path(args.zephyr_root).resolve()
    if not repo_root.is_dir():
        print(f"Error: {repo_root} not found")
        sys.exit(1)

    categories = [c.strip() for c in args.categories.split(",")]
    print(f"Zephyr root: {repo_root}")
    print(f"Categories: {categories}")

    entries = scan_tests(repo_root, categories)

    # Deduplicate by test_function + test_base + test_file
    seen = set()
    unique = []
    for e in entries:
        key = (e["test_function"], e["test_base"], e["test_file"])
        if key not in seen:
            seen.add(key)
            unique.append(e)

    unique.sort(key=lambda x: (x["test_category"], x["test_base"], x["test_function"]))

    # Output
    output_file = Path(args.output)
    output_file.parent.mkdir(parents=True, exist_ok=True)
    with open(output_file, "w", encoding="utf-8") as f:
        for e in unique:
            f.write(json.dumps(e, ensure_ascii=False) + "\n")

    # Summary
    cats = Counter(e["test_category"] for e in unique)
    suites = Counter(e["test_base"] for e in unique)
    print(f"\n{'='*60}")
    print(f"Extracted {len(unique)} unique test entries ({len(suites)} suites)")
    for cat, count in cats.most_common():
        print(f"  {cat:10s}: {count:>5d} entries")
    print(f"\nTop suites by test count:")
    for s, c in suites.most_common(15):
        print(f"  {s:30s} {c:>4d} tests")
    print(f"\nOutput: {output_file}")


if __name__ == "__main__":
    main()
