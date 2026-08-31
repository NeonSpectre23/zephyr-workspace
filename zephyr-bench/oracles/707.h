static inline k_ticks_t zbus_chan_pub_stats_last_time(const struct zbus_channel *chan)
{
	__ASSERT(chan != NULL, "chan is required");

	return chan->data->publish_timestamp;
}