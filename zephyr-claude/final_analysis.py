import json
from pathlib import Path

BENCH = Path("../zephyr-bench")

results = {}
with open("results/results.jsonl") as f:
    for line in f:
        if not line.strip():
            continue
        r = json.loads(line)
        tid = r["task_id"]
        if tid not in results or not results[tid].get("passed"):
            results[tid] = r

tasks = {}
for fn in ["zephyr_tasks.c.jsonl", "zephyr_tasks.h.jsonl"]:
    with open(BENCH / fn) as f:
        for line in f:
            if not line.strip():
                continue
            d = json.loads(line)
            tasks[d["task_id"]] = d

def get_traj(tid):
    p = Path(f"trajectory/task_{tid}.log")
    if not p.exists():
        return "", "", ""
    c = p.read_text(errors="replace")
    diff = ""
    verify = ""
    claude_out = ""
    if "=== Diff ===" in c:
        diff = c.split("=== Diff ===")[1].split("===")[0].strip()
    if "=== Harness Verify ===" in c:
        verify = c.split("=== Harness Verify ===")[1].strip()
    if "=== Claude Session ===" in c:
        claude_out = c.split("=== Claude Session ===")[1].split("===")[0].strip()
    return diff, verify, claude_out

def read_oracle(path):
    p = BENCH / path
    if p.exists():
        return p.read_text(errors="replace").strip()
    return "(oracle missing)"

def show_case(tid, title, explanation):
    r = results.get(tid)
    t = tasks.get(tid)
    if not r or not t:
        return
    diff, verify, claude = get_traj(tid)

    print(f"\n{'='*70}")
    print(f"  {title}")
    print(f"  Task {tid}: {t['sut_function']}")
    print(f"  Source: {t['source_path']}")
    print(f"  Error: {r.get('error_category','?')} | Time: {r.get('runtime_s',0)}s")
    print(f"{'='*70}")

    # Step 1: What Claude saw
    masked = t.get("masked_code", "").strip()
    print("\n  [Step 1: Claude saw this prompt]")
    print(f"  {'─'*60}")
    for line in masked.split("\n"):
        print(f"  | {line}")

    # Step 2: What Claude wrote (diff)
    if diff:
        print("\n  [Step 2: Claude wrote this code]")
        print(f"  {'─'*60}")
        # Show only added lines
        for line in diff.split("\n"):
            if line.startswith("+") and not line.startswith("+++"):
                print(f"  | {line}")
            elif line.startswith("-") and not line.startswith("---"):
                print(f"  | {line[:100]}")
    else:
        print("\n  [Step 2: Claude wrote]")
        print("  (diff not captured in trajectory)")

    # Step 3: What went wrong
    if verify:
        errors = []
        for line in verify.split("\n"):
            low = line.lower()
            if any(kw in low for kw in ["error:", "undefined", "undeclared",
                "fatal", "segmentation fault", "panic", "fail - ",
                "assertion failed"]):
                errors.append(line.strip()[:150])
        if errors:
            print("\n  [Step 3: Build/Test Errors]")
            print(f"  {'─'*60}")
            for e in errors[:5]:
                print(f"  > {e}")
        else:
            print("\n  [Step 3: No specific error in log]")
            last = verify.strip().split("\n")[-3:]
            for line2 in last:
                print(f"  {line2.strip()[:120]}")

    # Step 4: Correct answer
    oracle = read_oracle(t.get("oracle", ""))
    if oracle:
        print("\n  [Step 4: Oracle (correct answer)]")
        print(f"  {'─'*60}")
        for line in oracle.split("\n")[:12]:
            print(f"  | {line}")
        if len(oracle.split("\n")) > 12:
            print(f"  | ... ({len(oracle.split('\n'))} lines total)")

    # Step 5: Why it failed
    print("\n  [Why it failed]")
    print(f"  {explanation}")


# ================================================================
# 1. COMPILE_ERROR — undeclared (conditionally compiled out)
# ================================================================
show_case("146",
    "COMPILE_ERROR: undeclared — 函数被预处理器删掉了 (2 cases)",
    "Claude 看到了 log_core.c 里同时定义了 default_get_timestamp() 和\n"
    "default_rt_get_timestamp(), 但它俩分属 #ifndef / #else 两个互斥分支。\n"
    "Claude 用运行时 if-else 同时引用两个函数, 但当前 Kconfig 只启用了\n"
    "#ifndef 分支, 预处理器把 default_rt_get_timestamp 整段删掉了。\n"
    "编译器提示 did you mean 'default_lf_get_timestamp'? 正好印证:\n"
    "Claude 的逻辑语义上是对的, 但它不知道当前构建配置下哪个符号可见。")

# ================================================================
# 2. COMPILE_ERROR — undefined reference
# ================================================================
show_case("64",
    "COMPILE_ERROR: undefined reference — 调用了不存在的函数 (5 cases)",
    "fcb_walk() 需要初始化 flash 区域。Claude 猜函数名叫 fcb_open_flash(),\n"
    "但实际 API 是 flash_area_open() — 它来自 Zephyr 的 flash 子系统,\n"
    "命名规则是 flash_area_xxx, 不是 fcb_xxx。Claude 根据周围的 fcb_ 前缀\n"
    "推断错了。GCC 在链接阶段才报错: undefined reference to `fcb_open_flash'。\n"
    "函数签名看起来合理, 所以编译阶段没有报 warning, 到链接才炸。")

# ================================================================
# 3. COMPILE_ERROR — syntax / preprocessor
# ================================================================
show_case("262",
    "COMPILE_ERROR: syntax — 打破了 #if/#endif 配对 (3 cases)",
    "net_pkt.c 里这个函数体内嵌了 #if/#else/#endif 预处理块。\n"
    "Claude 插入的新代码里用到了其中定义的变量, 但写入的位置打断了\n"
    "#if 和 #else 的配对关系。编译时 GCC 报 unterminated #else ——\n"
    "这是因为 Claude 不把预处理器看作结构边界, 只当它是普通文本。")

# ================================================================
# 4. TEST_FAILURE — CRC implementation
# ================================================================
show_case("36",
    "TEST_FAILURE: CRC 查表算法写错 (28 cases)",
    "crc32_ieee() 是一个标准的 CRC-32 实现。Claude 选择了正确的策略\n"
    "(调用底层 crc32_ieee_update), 但传入的初始值不对:\n"
    "Claude 用了 0xFFFFFFFFU 作为初始 CRC 值, 但正确的调用是在 update\n"
    "之后还需要对结果取反 (~crc)。少了一步就导致所有 CRC 值都错了。\n"
    "这些测试用例用固定输入验证固定输出, 算法差一位就全 FAIL。")

# ================================================================
# 5. CRASH — segfault at runtime
# ================================================================
show_case("60",
    "CRASH: segfault — 访问了空指针 (16 cases)",
    "fcb_init() 需要初始化一个 flash 文件系统实例。Claude 写了\n"
    "对 fcbp 结构体各字段的赋值逻辑, 但有一行写的是\n"
    "fcbp->fap = fcbp->fap; (把指针赋值给自己)。\n"
    "这个字段从未被初始化, 后续代码通过这个 NULL 指针访问 flash\n"
    "→  Segmentation fault。代码编译没问题, 运行时一碰就崩。")

# ================================================================
# 6. ILLEGAL — edited outside function
# ================================================================
show_case("10",
    "ILLEGAL: AST violation — 函数体外加了 #undef (10 cases)",
    "cbprintf_packaged.c 里这个函数使用了 #define STR_POS_RO_FLAG\n"
    "等几个宏, 它们是文件级别的预处理宏, 不是函数体的一部分。\n"
    "Claude 在写完函数体后, 额外加了 #undef STR_POS_RO_FLAG 等行\n"
    "清理这些宏。虽然逻辑合理(用了就清理), 但这些 #undef 写在\n"
    "函数结束的 } 之外, 属于改了函数外代码。\n"
    "框架的 AST 归一化校验正确捕获了: 去掉函数体后, 两份代码不一致。")

# ================================================================
# 7. FRAMEWORK — timeout
# ================================================================
print(f"\n{'='*70}")
print("  FRAMEWORK BUG: timeout (6 cases)")
print("  代表: task 55, 130, 236, 237, 253, 552")
print(f"{'='*70}")
print("  west build 在 5 分钟超时。一般是该模块依赖太多或设备树太复杂,")
print("  在 native_sim 上编译特别慢。不是 Claude 的问题。")

# ================================================================
# 8. FRAMEWORK — UTF-8 decode
# ================================================================
print(f"\n{'='*70}")
print("  FRAMEWORK BUG: UTF-8 decode (5 cases)")
print("  代表: task 77, 86, 88, 91, 92")
print(f"{'='*70}")
print("  ninja/cmark 输出里夹了二进制字符, Python 的 text=True 解析失败。")
print("  纯框架 bug, Claude 的代码不一定有错。")
