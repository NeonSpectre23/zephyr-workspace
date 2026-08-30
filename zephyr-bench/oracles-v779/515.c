int zbus_chan_finish(const struct zbus_channel *chan)
{
	_ZBUS_ASSERT(chan != NULL, "chan is required");

	k_sem_give(&chan->data->sem);

	return 0;
}