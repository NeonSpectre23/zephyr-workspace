int mpsc_pbuf_get_max_utilization(struct mpsc_pbuf_buffer *buffer, uint32_t *max)
{
	int rc;
	k_spinlock_key_t key = k_spin_lock(&buffer->lock);

	if (buffer->flags & MPSC_PBUF_MAX_UTILIZATION) {
		*max = buffer->max_usage * sizeof(int);
		rc = 0;
	} else {
		rc = -ENOTSUP;
	}

	k_spin_unlock(&buffer->lock, key);

	return rc;
}