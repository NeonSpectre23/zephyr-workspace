int shell_set_root_cmd(const char *cmd)
{
	const struct shell_static_entry *entry;

	entry = cmd ? root_cmd_find(cmd) : NULL;

	if (cmd && (entry == NULL)) {
		return -EINVAL;
	}

	STRUCT_SECTION_FOREACH(shell, sh) {
		sh->ctx->selected_cmd = entry;
	}

	return 0;
}