static inline uint32_t zbus_chan_pub_stats_avg_period(const struct zbus_channel *chan)
{
	__ASSERT(chan != NULL, "chan is required");

	/* Not yet published, period = 0ms */
	if (chan->data->publish_count == 0) {
		return 0;
	}
	/* Average period across application runtime */
	return k_uptime_get() / chan->data->publish_count;
}