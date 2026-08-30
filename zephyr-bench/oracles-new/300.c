void mctp_uart_start_rx(struct mctp_binding_uart *uart)
{
	int res;

	/* Reset callback so binding which is doing RX is used */
	res = uart_callback_set(uart->dev, mctp_uart_callback, uart);
	__ASSERT_NO_MSG(res == 0);

	uart->rx_buf_used[0] = true;
	res = uart_rx_enable(uart->dev, uart->rx_buf[0], sizeof(uart->rx_buf[0]), 1000);
	__ASSERT_NO_MSG(res == 0);
}