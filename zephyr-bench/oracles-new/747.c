bool shell_ready(const struct shell *sh)
{
	__ASSERT_NO_MSG(sh);

	return state_get(sh) ==	SHELL_STATE_ACTIVE;
}

