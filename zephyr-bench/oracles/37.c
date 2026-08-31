int ec_host_cmd_init(struct ec_host_cmd_backend *backend)
{
	struct ec_host_cmd *hc = &ec_host_cmd;
	int ret;
	uint8_t *handler_tx_buf, *handler_rx_buf;
	uint8_t *handler_tx_buf_end, *handler_rx_buf_end;
	uint8_t *backend_tx_buf, *backend_rx_buf;

	hc->backend = backend;

	/* Allow writing to rx buff at startup */
	k_sem_init(&hc->rx_ready, 0, 1);

#ifdef CONFIG_EC_HOST_CMD_IN_PROGRESS_STATUS
	k_work_init(&work_in_progress, handler_in_progress);
#endif /* CONFIG_EC_HOST_CMD_IN_PROGRESS_STATUS */

	handler_tx_buf = hc->tx.buf;
	handler_rx_buf = hc->rx_ctx.buf;
	handler_tx_buf_end = handler_tx_buf + CONFIG_EC_HOST_CMD_HANDLER_TX_BUFFER_SIZE;
	handler_rx_buf_end = handler_rx_buf + CONFIG_EC_HOST_CMD_HANDLER_RX_BUFFER_SIZE;

	ret = backend->api->init(backend, &hc->rx_ctx, &hc->tx);

	backend_tx_buf = hc->tx.buf;
	backend_rx_buf = hc->rx_ctx.buf;

	if (ret != 0) {
		return ret;
	}

	if (!backend_tx_buf || !backend_rx_buf) {
		LOG_ERR("No buffer for Host Command communication");
		return -EIO;
	}

	hc->state = EC_HOST_CMD_STATE_RECEIVING;

	/* Check if a backend uses provided buffers. The buffer pointers can be shifted within the
	 * buffer to make space for preamble. Make sure the rx/tx pointers are within the provided
	 * buffers ranges.
	 */
	if ((handler_tx_buf &&
	     !((handler_tx_buf <= backend_tx_buf) && (handler_tx_buf_end > backend_tx_buf))) ||
	    (handler_rx_buf &&
	     !((handler_rx_buf <= backend_rx_buf) && (handler_rx_buf_end > backend_rx_buf)))) {
		LOG_WRN("Host Command handler provided unused buffer");
	}

#ifdef CONFIG_EC_HOST_CMD_DEDICATED_THREAD
	k_thread_create(&hc->thread, hc_stack, CONFIG_EC_HOST_CMD_HANDLER_STACK_SIZE,
			ec_host_cmd_thread, (void *)hc, NULL, NULL, CONFIG_EC_HOST_CMD_HANDLER_PRIO,
			0, K_NO_WAIT);
	k_thread_name_set(&hc->thread, "ec_host_cmd");
#endif /* CONFIG_EC_HOST_CMD_DEDICATED_THREAD */

	return 0;
}