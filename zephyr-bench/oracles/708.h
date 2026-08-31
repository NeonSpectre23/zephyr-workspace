static inline uint64_t zbus_chan_pub_stats_msg_age(const struct zbus_channel *chan)
{
	if (zbus_chan_pub_stats_count(chan) == 0) {
		return UINT64_MAX;
	}
	return k_ticks_to_ms_floor64(k_uptime_ticks() - zbus_chan_pub_stats_last_time(chan));
}