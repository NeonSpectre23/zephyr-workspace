int pbuf_read(struct pbuf *pb, char *buf, uint16_t len)
{
	if (pb == NULL) {
		/* Incorrect call. */
		return -EINVAL;
	}

	/* Invalidate wr_idx only, local rd_idx is used to increase buffer security. */
	sys_cache_data_invd_range((void *)(pb->cfg->wr_idx_loc), sizeof(*(pb->cfg->wr_idx_loc)));
	barrier_sync_synchronize();

	uint8_t *const data_loc = pb->cfg->data_loc;
	const uint32_t blen = pb->cfg->len;
	uint32_t wr_idx = *(pb->cfg->wr_idx_loc);
	uint32_t rd_idx = pb->data.rd_idx;

	/* rd_idx must always be aligned. */
	__ASSERT_NO_MSG(IS_PTR_ALIGNED_BYTES(rd_idx, _PBUF_IDX_SIZE));
	/* wr_idx shall always be aligned, but its value is received from the
	 * writer. Can not assert.
	 */
	if (!IS_PTR_ALIGNED_BYTES(wr_idx, _PBUF_IDX_SIZE)) {
		return -EINVAL;
	}

	if (rd_idx == wr_idx) {
		/* Buffer is empty. */
		return 0;
	}

	/* Get packet len.*/
	sys_cache_data_invd_range(&data_loc[rd_idx], PBUF_PACKET_LEN_SZ);
	uint16_t plen = sys_get_be16(&data_loc[rd_idx]);

	if (!buf) {
		return (int)plen;
	}

	if (plen > len) {
		return -ENOMEM;
	}

	uint32_t occupied_space = idx_occupied(blen, wr_idx, rd_idx);

	if (occupied_space < plen + PBUF_PACKET_LEN_SZ) {
		/* This should never happen. */
		return -EAGAIN;
	}

	rd_idx = idx_wrap(blen, rd_idx + PBUF_PACKET_LEN_SZ);

	/* Packet will fit into provided buffer, truncate len if provided len
	 * is bigger than necessary.
	 */
	len = MIN(plen, len);

	/* Read until end of the buffer, if data are wrapped. */
	uint32_t tail = MIN(blen - rd_idx, len);

	sys_cache_data_invd_range(&data_loc[rd_idx], tail);
	memcpy(buf, &data_loc[rd_idx], tail);

	if (len > tail) {
		sys_cache_data_invd_range(&data_loc[0], len - tail);
		memcpy(&buf[tail], &pb->cfg->data_loc[0], len - tail);
	}

	/* Update rd_idx. */
	rd_idx = idx_wrap(blen, ROUND_UP(rd_idx + len, _PBUF_IDX_SIZE));

	pb->data.rd_idx = rd_idx;
	*(pb->cfg->rd_idx_loc) = rd_idx;
	barrier_sync_synchronize();
	sys_cache_data_flush_range((void *)pb->cfg->rd_idx_loc, sizeof(*(pb->cfg->rd_idx_loc)));

	return len;
}