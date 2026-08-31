struct spsc_pbuf *spsc_pbuf_init(void *buf, size_t blen, uint32_t flags)
{
	if (!check_alignment(buf, flags)) {
		__ASSERT(false, "Failed to initialize due to memory misalignment");
		return NULL;
	}

	/* blen must be big enough to contain spsc_pbuf struct, byte of data
	 * and message len (2 bytes).
	 */
	struct spsc_pbuf *pb = buf;
	uint32_t *wr_idx_loc = get_wr_idx_loc(pb, flags);

	__ASSERT_NO_MSG(blen > (sizeof(*pb) + LEN_SZ));

	pb->common.len = get_len(blen, flags);
	pb->common.rd_idx = 0;
	pb->common.flags = flags;
	*wr_idx_loc = 0;

	barrier_sync_synchronize();
	cache_wb(&pb->common, sizeof(pb->common), flags);
	cache_wb(wr_idx_loc, sizeof(*wr_idx_loc), flags);

	return pb;
}