union mpsc_pbuf_generic *mpsc_pbuf_alloc(struct mpsc_pbuf_buffer *buffer,
					 size_t wlen, k_timeout_t timeout)
{
	union mpsc_pbuf_generic *item = NULL;
	union mpsc_pbuf_generic *dropped_item = NULL;
	bool cont = true;
	uint32_t free_wlen;
	uint32_t tmp_wr_idx_shift = 0;
	uint32_t tmp_wr_idx_val = 0;

	MPSC_PBUF_DBG(buffer, "alloc %d words", (int)wlen);

	if (wlen > (buffer->size)) {
		MPSC_PBUF_DBG(buffer, "Failed to alloc");
		return NULL;
	}

	do {
		k_spinlock_key_t key;
		bool wrap;

		key = k_spin_lock(&buffer->lock);
		if (tmp_wr_idx_shift) {
			post_drop_action(buffer, tmp_wr_idx_val, tmp_wr_idx_shift);
			tmp_wr_idx_shift = 0;
		}

		wrap = free_space(buffer, &free_wlen);

		if (free_wlen >= wlen) {
			item =
			    (union mpsc_pbuf_generic *)&buffer->buf[buffer->tmp_wr_idx];
			item->hdr.valid = 0;
			item->hdr.busy = 0;
			tmp_wr_idx_inc(buffer, wlen);
			cont = false;
		} else if (wrap) {
			add_skip_item(buffer, free_wlen);
			cont = true;
		} else if (IS_ENABLED(CONFIG_MULTITHREADING) && !K_TIMEOUT_EQ(timeout, K_NO_WAIT) &&
			   !k_is_in_isr() && arch_irq_unlocked(key.key)) {
			int err;

			k_spin_unlock(&buffer->lock, key);
			err = k_sem_take(&buffer->sem, timeout);
			key = k_spin_lock(&buffer->lock);
			cont = (err == 0) ? true : false;
		} else if (cont) {
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


	MPSC_PBUF_DBG(buffer, "allocated %p", item);

	if (IS_ENABLED(CONFIG_MPSC_CLEAR_ALLOCATED) && item) {
		/* During test fill with 0's to simplify message comparison */
		memset(item, 0, sizeof(int) * wlen);
	}

	return item;
}