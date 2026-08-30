uint32_t uart_emul_flush_tx_data(const struct device *dev)
{
	struct uart_emul_data *drv_data = dev->data;
	k_spinlock_key_t key;
	uint32_t count;

	key = k_spin_lock(&drv_data->tx_lock);
	count = ring_buf_size_get(drv_data->tx_rb);
	ring_buf_reset(drv_data->tx_rb);
	k_spin_unlock(&drv_data->tx_lock, key);
	return count;
}