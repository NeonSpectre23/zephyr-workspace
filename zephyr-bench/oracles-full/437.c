int shell_readline(const struct shell *sh, uint8_t *buf, size_t len, k_timeout_t timeout)
{
	k_timepoint_t end = sys_timepoint_calc(timeout);
	int ret;

	__ASSERT_NO_MSG(sh != NULL);

	/* Only allow calling from inside a shell command with no bypass active */
	if (!z_flag_cmd_ctx_get(sh) || sh->ctx->bypass != NULL) {
		return -EACCES;
	}

	sh->ctx->readline_state = SHELL_READLINE_ACTIVE;

	/* Save the current command buffer */
	sh->ctx->cmd_tmp_buff_len = sh->ctx->cmd_buff_len;
	sh->ctx->cmd_tmp_buff_pos = sh->ctx->cmd_buff_pos;
	memcpy(sh->ctx->temp_buff, sh->ctx->cmd_buff, sh->ctx->cmd_buff_len);

	/* Clear the buffer for user input */
	cmd_buffer_clear(sh);

	while (true) {
		state_collect(sh);

		if (sh->ctx->readline_state == SHELL_READLINE_DONE) {
			if (buf == NULL || sh->ctx->cmd_buff_len >= len) {
				ret = -ENOBUFS;
				break;
			}

			memcpy(buf, sh->ctx->cmd_buff, sh->ctx->cmd_buff_len);
			buf[sh->ctx->cmd_buff_len] = '\0';

			ret = sh->ctx->cmd_buff_len;
			break;
		}

		if (sh->ctx->readline_state == SHELL_READLINE_CANCELED) {
			ret = -ECANCELED;
			break;
		}

		/* Check for timeout */
		if (sys_timepoint_expired(end)) {
			ret = -ETIMEDOUT;
			break;
		}

		/* Small delay to avoid busy-waiting */
		k_msleep(1);
	}

	/* Restore the command state */
	sh->ctx->cmd_buff_len = sh->ctx->cmd_tmp_buff_len;
	sh->ctx->cmd_buff_pos = sh->ctx->cmd_tmp_buff_pos;
	memcpy(sh->ctx->cmd_buff, sh->ctx->temp_buff, sh->ctx->cmd_buff_len);

	sh->ctx->readline_state = SHELL_READLINE_INACTIVE;
	return ret;
}