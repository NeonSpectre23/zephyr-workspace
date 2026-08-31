const union mpsc_pbuf_generic *mpsc_pbuf_claim(struct mpsc_pbuf_buffer *buffer)
{
	union mpsc_pbuf_generic *item;
	bool cont;
	bool need_post = false;

	do {
		uint32_t a;
		k_spinlock_key_t key;

		cont = false;
		key = k_spin_lock(&buffer->lock);
		(void)available(buffer, &a);
		item = (union mpsc_pbuf_generic *)
			&buffer->buf[buffer->tmp_rd_idx];

		if (!a || is_invalid(item)) {
			MPSC_PBUF_DBG(buffer, "invalid claim %d: %p", a, item);
			item = NULL;
		} else {
			uint32_t skip = get_skip(item);

			if (skip || !is_valid(item)) {
				uint32_t inc =
					skip ? skip : buffer->get_wlen(item);

				buffer->tmp_rd_idx =
				      idx_inc(buffer, buffer->tmp_rd_idx, inc);
				rd_idx_inc(buffer, inc);
				cont = true;
				need_post = true;
			} else {
				item->hdr.busy = 1;
				buffer->tmp_rd_idx =
					idx_inc(buffer, buffer->tmp_rd_idx,
						buffer->get_wlen(item));
			}
		}

		if (!cont) {
			MPSC_PBUF_DBG(buffer, ">>claimed %d: %p", a, item);
		}
		k_spin_unlock(&buffer->lock, key);
	} while (cont);

	if (IS_ENABLED(CONFIG_MULTITHREADING) && need_post && item == NULL) {
		k_sem_give(&buffer->sem);
	}

	return item;
}