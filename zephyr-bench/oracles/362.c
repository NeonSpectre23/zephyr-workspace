uint16_t spsc_pbuf_claim(struct spsc_pbuf *pb, char **buf)
{
	/* Length of the buffer and flags are immutable - avoid reloading. */
	const uint32_t pblen = pb->common.len;
	const uint32_t flags = pb->common.flags;
	uint32_t *rd_idx_loc = get_rd_idx_loc(pb, flags);
	uint32_t *wr_idx_loc = get_wr_idx_loc(pb, flags);
	uint8_t *data_loc = get_data_loc(pb, flags);

	cache_inv(wr_idx_loc, sizeof(*wr_idx_loc), flags);
	barrier_sync_synchronize();

	uint32_t wr_idx = *wr_idx_loc;
	uint32_t rd_idx = *rd_idx_loc;

	if (rd_idx == wr_idx) {
		return 0;
	}

	uint32_t bytes_stored = idx_occupied(pblen, wr_idx, rd_idx);

	/* Utilization is calculated at claiming to handle cache case when flags
	 * and rd_idx is in the same cache line thus it should be modified only
	 * by the consumer.
	 */
	if (IS_ENABLED(CONFIG_SPSC_PBUF_UTILIZATION) && (bytes_stored > GET_UTILIZATION(flags))) {
		__ASSERT_NO_MSG(bytes_stored <= BIT_MASK(SPSC_PBUF_UTILIZATION_BITS));
		pb->common.flags = SET_UTILIZATION(flags, bytes_stored);
		barrier_sync_synchronize();
		cache_wb(&pb->common.flags, sizeof(pb->common.flags), flags);
	}

	/* Read message len. */
	uint16_t len;

	cache_inv(&data_loc[rd_idx], LEN_SZ, flags);
	if (data_loc[rd_idx] == PADDING_MARK) {
		/* If padding is found we must check if we are interrupted
		 * padding injection procedure which has 2 steps (adding padding,
		 * changing write index). If padding is added but index is not
		 * yet changed, it indicates that there is no data after the
		 * padding (at the beginning of the buffer).
		 */
		cache_inv(wr_idx_loc, sizeof(*wr_idx_loc), flags);
		if (rd_idx == *wr_idx_loc) {
			return 0;
		}

		*rd_idx_loc = rd_idx = 0;
		barrier_sync_synchronize();
		cache_wb(rd_idx_loc, sizeof(*rd_idx_loc), flags);
		/* After reading padding we may find out that buffer is empty. */
		if (rd_idx == wr_idx) {
			return 0;
		}

		cache_inv(&data_loc[rd_idx], sizeof(len), flags);
	}

	len = sys_get_be16(&data_loc[rd_idx]);

	(void)bytes_stored;
	__ASSERT_NO_MSG(bytes_stored >= (len + LEN_SZ));

	cache_inv(&data_loc[rd_idx + LEN_SZ], len, flags);
	*buf = &data_loc[rd_idx + LEN_SZ];

	return len;
}