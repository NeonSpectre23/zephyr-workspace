uint32_t uart_emul_get_tx_data(const struct device *dev, uint8_t *data, size_t size)
{
	struct uart_emul_data *drv_data = dev->data;
	k_spinlock_key_t key;
	uint32_t count;

	key = k_spin_lock(&drv_data->tx_lock);
	count = ring_buf_get(drv_data->tx_rb, data, size);
	k_spin_unlock(&drv_data->tx_lock, key);
	return count;
}