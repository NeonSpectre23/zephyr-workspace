void mpsc_pbuf_put_word(struct mpsc_pbuf_buffer *buffer,
			const union mpsc_pbuf_generic item)
{
	bool cont;
	uint32_t free_wlen;
	k_spinlock_key_t key;
	union mpsc_pbuf_generic *dropped_item = NULL;
	uint32_t tmp_wr_idx_shift = 0;
	uint32_t tmp_wr_idx_val = 0;

	do {
		key = k_spin_lock(&buffer->lock);

		if (tmp_wr_idx_shift) {
			post_drop_action(buffer, tmp_wr_idx_val, tmp_wr_idx_shift);
			tmp_wr_idx_shift = 0;
		}

		(void)free_space(buffer, &free_wlen);

		MPSC_PBUF_DBG(buffer, "put_word (%d free space)", (int)free_wlen);

		if (free_wlen) {
			buffer->buf[buffer->tmp_wr_idx] = item.raw;
			tmp_wr_idx_inc(buffer, 1);
			cont = false;
			buffer->wr_idx = idx_inc(buffer, buffer->wr_idx, 1);
			max_utilization_update(buffer);
		} else {
			tmp_wr_idx_val = buffer->tmp_wr_idx;
			cont = drop_item_locked(buffer, free_wlen,
						&dropped_item, &tmp_wr_idx_shift);
		}

		k_spin_unlock(&buffer->lock, key);

		if (dropped_item) {
			/* Notify about item being dropped. */
			if (buffer->notify_drop) {
				buffer->notify_drop(buffer, dropped_item);
			}
			dropped_item = NULL;
		}
	} while (cont);
}