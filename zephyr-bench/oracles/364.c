void spsc_pbuf_free(struct spsc_pbuf *pb, uint16_t len)
{
	/* Length of the buffer and flags are immutable - avoid reloading. */
	const uint32_t pblen = pb->common.len;
	const uint32_t flags = pb->common.flags;
	uint32_t *rd_idx_loc = get_rd_idx_loc(pb, flags);
	uint32_t *wr_idx_loc = get_wr_idx_loc(pb, flags);
	uint16_t rd_idx = *rd_idx_loc + len + LEN_SZ;
	uint8_t *data_loc = get_data_loc(pb, flags);

	rd_idx = ROUND_UP(rd_idx, sizeof(uint32_t));
	/* Handle wrapping or the fact that next packet is a padding. */
	if (rd_idx != pblen) {
		cache_inv(&data_loc[rd_idx], sizeof(uint8_t), flags);
		if (data_loc[rd_idx] == PADDING_MARK) {
			cache_inv(wr_idx_loc, sizeof(*wr_idx_loc), flags);
			/* We may hit the case when producer is in the middle of adding
			 * a padding (which happens in 2 steps: writing padding, resetting
			 * write index) and in that case we cannot consume this padding.
			 */
			if (rd_idx != *wr_idx_loc) {
				rd_idx = 0;
			}
		}
	} else {
		rd_idx = 0;
	}

	*rd_idx_loc = rd_idx;
	barrier_sync_synchronize();
	cache_wb(rd_idx_loc, sizeof(*rd_idx_loc), flags);
}