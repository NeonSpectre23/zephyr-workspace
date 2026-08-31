struct modem_pipe *modem_backend_tty_init(struct modem_backend_tty *backend,
					  const struct modem_backend_tty_config *config)
{
	__ASSERT_NO_MSG(backend != NULL);
	__ASSERT_NO_MSG(config != NULL);
	__ASSERT_NO_MSG(config->tty_path != NULL);

	memset(backend, 0x00, sizeof(*backend));
	backend->tty_path = config->tty_path;
	backend->stack = config->stack;
	backend->stack_size = config->stack_size;
	atomic_set(&backend->state, 0);
	modem_pipe_init(&backend->pipe, backend, &modem_backend_tty_api);
	return &backend->pipe;
}