int zbus_chan_read(const struct zbus_channel *chan, void *msg, k_timeout_t timeout)
{
	_ZBUS_ASSERT(chan != NULL, "chan is required");
	_ZBUS_ASSERT(msg != NULL, "msg is required");
	_ZBUS_ASSERT(k_is_in_isr() ? K_TIMEOUT_EQ(timeout, K_NO_WAIT) : true,
		     "inside an ISR, the timeout must be K_NO_WAIT");

	if (k_is_in_isr()) {
		timeout = K_NO_WAIT;
	}

	int err = k_sem_take(&chan->data->sem, timeout);
	if (err) {
		return err;
	}

	memcpy(msg, chan->message, chan->message_size);

	k_sem_give(&chan->data->sem);

	return 0;
}