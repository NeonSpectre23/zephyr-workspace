void mpsc_pbuf_free(struct mpsc_pbuf_buffer *buffer,
		     const union mpsc_pbuf_generic *item)
{
	uint32_t wlen = buffer->get_wlen(item);
	k_spinlock_key_t key = k_spin_lock(&buffer->lock);
	union mpsc_pbuf_generic *witem = (union mpsc_pbuf_generic *)item;

	witem->hdr.valid = 0;
	if (!(buffer->flags & MPSC_PBUF_MODE_OVERWRITE) ||
		 ((uint32_t *)item == &buffer->buf[buffer->rd_idx])) {
		witem->hdr.busy = 0;
		if (buffer->rd_idx == buffer->tmp_rd_idx) {
			/* There is a chance that there are so many new packets
			 * added between claim and free that rd_idx points again
			 * at claimed item. In that case tmp_rd_idx points at
			 * the same location. In that case increment also tmp_rd_idx
			 * which will mark freed buffer as the only free space in
			 * the buffer.
			 */
			buffer->tmp_rd_idx = idx_inc(buffer, buffer->tmp_rd_idx, wlen);
		}
		rd_idx_inc(buffer, wlen);
	} else {
		MPSC_PBUF_DBG(buffer, "Allocation occurred during claim");
		witem->skip.len = wlen;
	}
	MPSC_PBUF_DBG(buffer, "<<freed: %p", item);

	k_spin_unlock(&buffer->lock, key);
	if (IS_ENABLED(CONFIG_MULTITHREADING)) {
		k_sem_give(&buffer->sem);
	}
}