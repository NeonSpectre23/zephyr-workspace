struct modem_pipe *modem_cmux_dlci_init(struct modem_cmux *cmux, struct modem_cmux_dlci *dlci,
					const struct modem_cmux_dlci_config *config)
{
	__ASSERT_NO_MSG(cmux != NULL);
	__ASSERT_NO_MSG(dlci != NULL);
	__ASSERT_NO_MSG(config != NULL);
	__ASSERT_NO_MSG(config->dlci_address < 64);
	__ASSERT_NO_MSG(config->receive_buf != NULL);
	__ASSERT_NO_MSG(config->receive_buf_size >= 126);

	memset(dlci, 0x00, sizeof(*dlci));
	dlci->cmux = cmux;
	dlci->dlci_address = config->dlci_address;
	ring_buf_init(&dlci->receive_rb, config->receive_buf_size, config->receive_buf);
	k_mutex_init(&dlci->receive_rb_lock);
	modem_pipe_init(&dlci->pipe, dlci, &modem_cmux_dlci_pipe_api);
	k_work_init_delayable(&dlci->open_work, modem_cmux_dlci_open_handler);
	k_work_init_delayable(&dlci->close_work, modem_cmux_dlci_close_handler);
	dlci->state = MODEM_CMUX_DLCI_STATE_CLOSED;
	sys_slist_append(&dlci->cmux->dlcis, &dlci->node);

#if CONFIG_MODEM_STATS
	modem_cmux_dlci_init_buf_stats(dlci);
#endif

	return &dlci->pipe;
}