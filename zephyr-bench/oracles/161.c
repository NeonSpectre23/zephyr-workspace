void modem_cmux_init(struct modem_cmux *cmux, const struct modem_cmux_config *config)
{
	__ASSERT_NO_MSG(cmux != NULL);
	__ASSERT_NO_MSG(config != NULL);
	__ASSERT_NO_MSG(config->receive_buf != NULL);
	__ASSERT_NO_MSG(config->receive_buf_size >= MODEM_CMUX_DATA_FRAME_SIZE_MAX);
	__ASSERT_NO_MSG(config->transmit_buf != NULL);
	__ASSERT_NO_MSG(config->transmit_buf_size >= MODEM_CMUX_DATA_FRAME_SIZE_MAX);

	*cmux = (struct modem_cmux){
		.t3_timepoint = sys_timepoint_calc(K_NO_WAIT),
		.config = *config,
	};
	sys_slist_init(&cmux->dlcis);
	ring_buf_init(&cmux->transmit_rb, cmux->config.transmit_buf_size,
		      cmux->config.transmit_buf);
	k_mutex_init(&cmux->transmit_rb_lock);
	k_work_init_delayable(&cmux->receive_work, modem_cmux_receive_handler);
	k_work_init_delayable(&cmux->transmit_work, modem_cmux_transmit_handler);
	k_work_init_delayable(&cmux->connect_work, modem_cmux_connect_handler);
	k_work_init_delayable(&cmux->disconnect_work, modem_cmux_disconnect_handler);
	k_work_init_delayable(&cmux->runtime_pm_work, modem_cmux_runtime_pm_handler);
	k_event_init(&cmux->event);
	set_state(cmux, MODEM_CMUX_STATE_DISCONNECTED);

#if CONFIG_MODEM_STATS
	modem_cmux_init_buf_stats(cmux);
#endif
}