void spsc_pbuf_commit(struct spsc_pbuf *pb, uint16_t len)
{
	if (len == 0) {
		return;
	}

	/* Length of the buffer and flags are immutable - avoid reloading. */
	const uint32_t pblen = pb->common.len;
	const uint32_t flags = pb->common.flags;
	uint32_t *wr_idx_loc = get_wr_idx_loc(pb, flags);
	uint8_t *data_loc = get_data_loc(pb, flags);

	uint32_t wr_idx = *wr_idx_loc;

	sys_put_be16(len, &data_loc[wr_idx]);
	barrier_sync_synchronize();
	cache_wb(&data_loc[wr_idx], len + LEN_SZ, flags);

	wr_idx += len + LEN_SZ;
	wr_idx = ROUND_UP(wr_idx, sizeof(uint32_t));
	wr_idx = wr_idx == pblen ? 0 : wr_idx;

	*wr_idx_loc = wr_idx;
	barrier_sync_synchronize();
	cache_wb(wr_idx_loc, sizeof(*wr_idx_loc), flags);
}