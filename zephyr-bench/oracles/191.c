bool mpsc_pbuf_is_pending(struct mpsc_pbuf_buffer *buffer)
{
	uint32_t a;
	k_spinlock_key_t key = k_spin_lock(&buffer->lock);

	(void)available(buffer, &a);
	k_spin_unlock(&buffer->lock, key);

	return a ? true : false;
}