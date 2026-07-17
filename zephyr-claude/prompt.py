"""
Prompt 模板 —— 发给 Claude Code CLI 的 Zephyr 任务描述。
"""
TASK_PROMPT = """Implement the missing function `{sut_function}` in `{source_path}`.

The function body is currently replaced with a TODO comment.

RULES:
- Only edit the file {source_path}
- Do NOT modify any other files (they are read-only)
- The code must compile with Zephyr's build system (west build + cmake + ninja)
- Use Zephyr RTOS APIs and conventions where appropriate
"""
