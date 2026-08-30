int lorawan_clock_sync_get(uint32_t *gps_time)
{
	__ASSERT(gps_time != NULL, "gps_time parameter is required");

	if (ctx.synchronized) {
		*gps_time = k_uptime_seconds() + ctx.time_offset;
		return 0;
	} else {
		return -EAGAIN;
	}
}
