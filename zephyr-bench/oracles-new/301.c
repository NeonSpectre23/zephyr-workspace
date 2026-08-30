int mctp_uart_stop_rx(struct mctp_binding_uart *uart)
{
	int res = uart_rx_disable(uart->dev);

	if (res != 0) {
		return res;
	}

	/* Ensure rx is fully stopped before returning */
	k_sem_take(&uart->rx_disabled, K_FOREVER);

	return 0;
}