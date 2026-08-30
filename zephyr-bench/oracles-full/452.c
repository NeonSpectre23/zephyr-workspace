int spsc_pbuf_get_utilization(struct spsc_pbuf *pb)
{
	if (!IS_ENABLED(CONFIG_SPSC_PBUF_UTILIZATION)) {
		return -ENOTSUP;
	}

	cache_inv(&pb->common.flags, sizeof(pb->common.flags), pb->common.flags);
	barrier_sync_synchronize();

	return GET_UTILIZATION(pb->common.flags);
}