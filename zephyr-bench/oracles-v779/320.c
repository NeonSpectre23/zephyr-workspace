int pbuf_write(struct pbuf *pb, const char *data, uint16_t len)
{
	if (pb == NULL || len == 0 || data == NULL) {
		/* Incorrect call. */
		return -EINVAL;
	}

	/* Invalidate rd_idx only, local wr_idx is used to increase buffer security. */
	sys_cache_data_invd_range((void *)(pb->cfg->rd_idx_loc), sizeof(*(pb->cfg->rd_idx_loc)));
	barrier_sync_synchronize();

	uint8_t *const data_loc = pb->cfg->data_loc;
	const uint32_t blen = pb->cfg->len;
	uint32_t rd_idx = *(pb->cfg->rd_idx_loc);
	uint32_t wr_idx = pb->data.wr_idx;

	/* wr_idx must always be aligned. */
	__ASSERT_NO_MSG(IS_PTR_ALIGNED_BYTES(wr_idx, _PBUF_IDX_SIZE));
	/* rd_idx shall always be aligned, but its value is received from the reader.
	 * Can not assert.
	 */
	if (!IS_PTR_ALIGNED_BYTES(rd_idx, _PBUF_IDX_SIZE)) {
		return -EINVAL;
	}

	uint32_t free_space = blen - idx_occupied(blen, wr_idx, rd_idx) - _PBUF_IDX_SIZE;

	/* Packet length, data + packet length size. */
	uint32_t plen = len + PBUF_PACKET_LEN_SZ;

	/* Check if packet will fit into the buffer. */
	if (free_space < plen) {
		return -ENOMEM;
	}

	/* Clear packet len with zeros and update. Clearing is done for possible versioning in the
	 * future. Writing is allowed now, because shared wr_idx value is updated at the very end.
	 */
	*((uint32_t *)(&data_loc[wr_idx])) = 0;
	sys_put_be16(len, &data_loc[wr_idx]);
	barrier_sync_synchronize();
	sys_cache_data_flush_range(&data_loc[wr_idx], PBUF_PACKET_LEN_SZ);

	wr_idx = idx_wrap(blen, wr_idx + PBUF_PACKET_LEN_SZ);

	/* Write until end of the buffer, if data will be wrapped. */
	uint32_t tail = MIN(len, blen - wr_idx);

	memcpy(&data_loc[wr_idx], data, tail);
	sys_cache_data_flush_range(&data_loc[wr_idx], tail);

	if (len > tail) {
		/* Copy remaining data to buffer front. */
		memcpy(&data_loc[0], data + tail, len - tail);
		sys_cache_data_flush_range(&data_loc[0], len - tail);
	}

	wr_idx = idx_wrap(blen, ROUND_UP(wr_idx + len, _PBUF_IDX_SIZE));
	/* Update wr_idx. */
	pb->data.wr_idx = wr_idx;
	*(pb->cfg->wr_idx_loc) = wr_idx;
	barrier_sync_synchronize();
	sys_cache_data_flush_range((void *)pb->cfg->wr_idx_loc, sizeof(*(pb->cfg->wr_idx_loc)));

	return len;
}