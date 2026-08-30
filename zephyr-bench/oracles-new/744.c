int shell_execute_cmd(const struct shell *sh, const char *cmd)
{
	uint16_t cmd_len = z_shell_strlen(cmd);
	int ret_val;

	if (cmd == NULL) {
		return -ENOEXEC;
	}

	if (cmd_len > (CONFIG_SHELL_CMD_BUFF_SIZE - 1)) {
		return -ENOMEM;
	}

	if (sh == NULL) {
#if defined(CONFIG_SHELL_BACKEND_DUMMY)
		sh = shell_backend_dummy_get_ptr();
#else
		return -EINVAL;
#endif
	}

	__ASSERT(!z_flag_cmd_ctx_get(sh), "Function cannot be called"
					  " from command context");

	strcpy(sh->ctx->cmd_buff, cmd);
	sh->ctx->cmd_buff_len = cmd_len;
	sh->ctx->cmd_buff_pos = cmd_len;

	if (!z_shell_trylock(sh, SHELL_TX_MTX_TIMEOUT)) {
		return -ENOEXEC;
	}
	ret_val = execute(sh);
	z_shell_unlock(sh);

	cmd_buffer_clear(sh);

	return ret_val;
}

