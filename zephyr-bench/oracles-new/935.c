uint32_t uart_emul_flush_rx_data(const struct device *dev)
{
	struct uart_emul_data *drv_data = dev->data;
	k_spinlock_key_t key;
	uint32_t count;

	key = k_spin_lock(&drv_data->rx_lock);
	count = ring_buf_size_get(drv_data->rx_rb);
	ring_buf_reset(drv_data->rx_rb);
	k_spin_unlock(&drv_data->rx_lock, key);
	return count;
}