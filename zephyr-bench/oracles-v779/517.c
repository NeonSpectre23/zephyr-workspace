int zbus_chan_notify(const struct zbus_channel *chan, k_timeout_t timeout)
{
	int err;

	_ZBUS_ASSERT(chan != NULL, "chan is required");
	_ZBUS_ASSERT(k_is_in_isr() ? K_TIMEOUT_EQ(timeout, K_NO_WAIT) : true,
		     "inside an ISR, the timeout must be K_NO_WAIT");

	if (k_is_in_isr()) {
		timeout = K_NO_WAIT;
	}

	k_timepoint_t end_time = sys_timepoint_calc(timeout);

	int context_priority = ZBUS_MIN_THREAD_PRIORITY;

	err = chan_lock(chan, timeout, &context_priority);
	if (err) {
		return err;
	}

	err = _zbus_vded_exec(chan, end_time);

	chan_unlock(chan, context_priority);

	return err;
}