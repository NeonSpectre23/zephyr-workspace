static inline const char *zbus_chan_name(const struct zbus_channel *chan)
{
	__ASSERT(chan != NULL, "chan is required");

	return chan->name;
}