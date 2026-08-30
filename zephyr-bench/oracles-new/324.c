struct modem_pipe *modem_backend_uart_init(struct modem_backend_uart *backend,
					   const struct modem_backend_uart_config *config)
{
	__ASSERT_NO_MSG(config->uart != NULL);
	__ASSERT_NO_MSG(config->receive_buf != NULL);
	__ASSERT_NO_MSG(config->receive_buf_size > 1);
	__ASSERT_NO_MSG((config->receive_buf_size % 2) == 0);
	__ASSERT_NO_MSG(config->transmit_buf != NULL);
	__ASSERT_NO_MSG(config->transmit_buf_size > 0);

	memset(backend, 0x00, sizeof(*backend));
	backend->uart = config->uart;
	backend->dtr_gpio = config->dtr_gpio;
	k_work_init_delayable(&backend->receive_ready_work,
			      modem_backend_uart_receive_ready_handler);
	k_work_init(&backend->transmit_idle_work, modem_backend_uart_transmit_idle_handler);

#ifdef CONFIG_MODEM_BACKEND_UART_ASYNC
	if (modem_backend_uart_async_is_supported(backend)) {
		if (modem_backend_uart_async_init(backend, config)) {
			return NULL;
		}
		return &backend->pipe;
	}
#endif /* CONFIG_MODEM_BACKEND_UART_ASYNC */

#ifdef CONFIG_MODEM_BACKEND_UART_ISR
	modem_backend_uart_isr_init(backend, config);

	return &backend->pipe;
#endif /* CONFIG_MODEM_BACKEND_UART_ISR */

	__ASSERT(0, "No supported UART API");

	return NULL;
}