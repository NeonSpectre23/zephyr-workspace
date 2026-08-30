void shell_set_bypass(const struct shell *sh, shell_bypass_cb_t bypass, void *user_data)
{
	__ASSERT_NO_MSG(sh);

	sh->ctx->bypass = bypass;
	sh->ctx->bypass_user_data = user_data;

	if (bypass == NULL) {
		cmd_buffer_clear(sh);
	}
}