void mpsc_pbuf_commit(struct mpsc_pbuf_buffer *buffer,
		       union mpsc_pbuf_generic *item)
{
	uint32_t wlen = buffer->get_wlen(item);

	k_spinlock_key_t key = k_spin_lock(&buffer->lock);

	item->hdr.valid = 1;
	buffer->wr_idx = idx_inc(buffer, buffer->wr_idx, wlen);
	max_utilization_update(buffer);
	k_spin_unlock(&buffer->lock, key);
	MPSC_PBUF_DBG(buffer, "committed %p", item);
}