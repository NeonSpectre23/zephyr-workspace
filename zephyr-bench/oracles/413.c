uint32_t uart_emul_put_rx_data(const struct device *dev, const uint8_t *data, size_t size)
{
	struct uart_emul_data *drv_data = dev->data;
	uint32_t count = 0;
	__unused bool empty = false;
	__unused bool irq_en = false;
	__unused bool rx_en = false;

	K_SPINLOCK(&drv_data->rx_lock) {
		count = ring_buf_put(drv_data->rx_rb, data, size);
		empty = ring_buf_is_empty(drv_data->rx_rb);
		IF_ENABLED(CONFIG_UART_INTERRUPT_DRIVEN, (irq_en = drv_data->rx_irq_en;));
		IF_ENABLED(CONFIG_UART_ASYNC_API, (rx_en = drv_data->rx_async_en;));
	}

	if (count < size) {
		uart_emul_set_errors(dev, UART_ERROR_OVERRUN);
	}

	IF_ENABLED(CONFIG_UART_INTERRUPT_DRIVEN, (
		if (count > 0 && irq_en && !empty) {
			(void)k_work_submit_to_queue(&uart_emul_work_q, &drv_data->irq_work);
		}
	))
	IF_ENABLED(CONFIG_UART_ASYNC_API, (
		if (count > 0 && rx_en && !empty) {
			(void)k_work_submit_to_queue(&uart_emul_work_q, &drv_data->rx_work);
		}
	))

	return count;
}