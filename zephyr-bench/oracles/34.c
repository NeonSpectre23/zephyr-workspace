struct ec_host_cmd_backend *ec_host_cmd_backend_get_uart(const struct device *dev)
{
	struct ec_host_cmd_uart_ctx *hc_uart = ec_host_cmd_uart.ctx;

	hc_uart->uart_dev = dev;
	return &ec_host_cmd_uart;
}