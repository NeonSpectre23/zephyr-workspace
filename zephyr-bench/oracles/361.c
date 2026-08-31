int spsc_pbuf_alloc(struct spsc_pbuf *pb, uint16_t len, char **buf)
{
	/* Length of the buffer and flags are immutable - avoid reloading. */
	const uint32_t pblen = pb->common.len;
	const uint32_t flags = pb->common.flags;
	uint32_t *rd_idx_loc = get_rd_idx_loc(pb, flags);
	uint32_t *wr_idx_loc = get_wr_idx_loc(pb, flags);
	uint8_t *data_loc = get_data_loc(pb, flags);

	uint32_t space = len + LEN_SZ; /* data + length field */

	if (len == 0 || len > SPSC_PBUF_MAX_LEN) {
		/* Incorrect call. */
		return -EINVAL;
	}

	cache_inv(rd_idx_loc, sizeof(*rd_idx_loc), flags);
	barrier_sync_synchronize();

	uint32_t wr_idx = *wr_idx_loc;
	uint32_t rd_idx = *rd_idx_loc;
	int32_t free_space;

	if (wr_idx >= rd_idx) {
		int32_t remaining = pblen - wr_idx;
		/* If SPSC_PBUF_MAX_LEN is set as length try to allocate maximum
		 * possible packet till wrap or from the beginning.
		 * If len is bigger than SPSC_PBUF_MAX_LEN then try to allocate
		 * maximum packet length even if that results in adding a padding.
		 */
		if (len == SPSC_PBUF_MAX_LEN) {
			/* At least space for 1 byte packet. */
			space = LEN_SZ + 1;
		}

		if ((remaining >= space) || (rd_idx <= space)) {
			/* Packet will fit at the end. Free space depends on
			 * presence of data at the beginning of the buffer since
			 * there must be one word not used to distinguish between
			 * empty and full state.
			 */
			free_space = remaining - ((rd_idx > 0) ? 0 : FREE_SPACE_DISTANCE);
		} else {
			/* Padding must be added. */
			data_loc[wr_idx] = PADDING_MARK;
			barrier_sync_synchronize();
			cache_wb(&data_loc[wr_idx], sizeof(uint8_t), flags);

			wr_idx = 0;
			*wr_idx_loc = wr_idx;

			/* Obligatory one word empty space. */
			free_space = rd_idx - FREE_SPACE_DISTANCE;
		}
	} else {
		/* Obligatory one word empty space. */
		free_space = rd_idx - wr_idx - FREE_SPACE_DISTANCE;
	}

	len = min(len, max(free_space - (int32_t)LEN_SZ, 0));
	*buf = &data_loc[wr_idx + LEN_SZ];

	return len;
}