static inline void *zbus_chan_user_data(const struct zbus_channel *chan)
{
	__ASSERT(chan != NULL, "chan is required");

	return chan->user_data;
}