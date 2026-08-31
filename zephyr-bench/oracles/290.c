int pbuf_tx_init(struct pbuf *pb)
{
	if (validate_cfg(pb->cfg) != 0) {
		return -EINVAL;
	}
#if defined(CONFIG_ARCH_POSIX)
	pbuf_native_addr_remap(pb);
#endif

	/* Initialize local copy of indexes. */
	pb->data.wr_idx = 0;
	pb->data.rd_idx = 0;

	/* Clear shared memory. */
	*(pb->cfg->wr_idx_loc) = pb->data.wr_idx;
	*(pb->cfg->rd_idx_loc) = pb->data.rd_idx;

	barrier_sync_synchronize();

	/* Take care cache. */
	sys_cache_data_flush_range((void *)(pb->cfg->wr_idx_loc), sizeof(*(pb->cfg->wr_idx_loc)));
	sys_cache_data_flush_range((void *)(pb->cfg->rd_idx_loc), sizeof(*(pb->cfg->rd_idx_loc)));

	return 0;
}