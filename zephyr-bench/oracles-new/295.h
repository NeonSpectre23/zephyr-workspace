static inline const void *zbus_chan_const_msg(const struct zbus_channel *chan)
{
	__ASSERT(chan != NULL, "chan is required");

	return chan->message;
}