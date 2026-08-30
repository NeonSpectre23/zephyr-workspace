void mpsc_pbuf_get_utilization(struct mpsc_pbuf_buffer *buffer,
			       uint32_t *size, uint32_t *now)
{
	k_spinlock_key_t key = k_spin_lock(&buffer->lock);

	/* One byte is left for full/empty distinction. */
	*size = (buffer->size - 1) * sizeof(int);
	*now = get_usage(buffer) * sizeof(int);

	k_spin_unlock(&buffer->lock, key);
}