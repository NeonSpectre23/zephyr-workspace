void mpsc_pbuf_put_word_ext(struct mpsc_pbuf_buffer *buffer,
			    const union mpsc_pbuf_generic item,
			    const void *data)
{
	static const size_t l =
		(sizeof(item) + sizeof(data)) / sizeof(uint32_t);
	union mpsc_pbuf_generic *dropped_item = NULL;
	bool cont;
	uint32_t tmp_wr_idx_shift = 0;
	uint32_t tmp_wr_idx_val = 0;

	do {
		k_spinlock_key_t key;
		uint32_t free_wlen;
		bool wrap;

		key = k_spin_lock(&buffer->lock);

		if (tmp_wr_idx_shift) {
			post_drop_action(buffer, tmp_wr_idx_val, tmp_wr_idx_shift);
			tmp_wr_idx_shift = 0;
		}

		wrap = free_space(buffer, &free_wlen);

		if (free_wlen >= l) {
			buffer->buf[buffer->tmp_wr_idx] = item.raw;
			void **p =
				(void **)&buffer->buf[buffer->tmp_wr_idx + 1];

			*p = (void *)data;
			tmp_wr_idx_inc(buffer, l);
			buffer->wr_idx = idx_inc(buffer, buffer->wr_idx, l);
			cont = false;
			max_utilization_update(buffer);
		} else if (wrap) {
			add_skip_item(buffer, free_wlen);
			cont = true;
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