static inline uint32_t zbus_chan_pub_stats_count(const struct zbus_channel *chan)
{
	__ASSERT(chan != NULL, "chan is required");

	return chan->data->publish_count;
}