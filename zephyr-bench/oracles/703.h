static inline void *zbus_chan_msg(const struct zbus_channel *chan)
{
	__ASSERT(chan != NULL, "chan is required");

	return chan->message;
}