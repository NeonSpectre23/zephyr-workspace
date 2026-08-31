int zbus_sub_wait(const struct zbus_observer *sub, const struct zbus_channel **chan,
		  k_timeout_t timeout)
{
	_ZBUS_ASSERT(!k_is_in_isr(), "zbus_sub_wait cannot be used inside ISRs");
	_ZBUS_ASSERT(sub != NULL, "sub is required");
	_ZBUS_ASSERT(sub->type == ZBUS_OBSERVER_SUBSCRIBER_TYPE, "sub must be a SUBSCRIBER");
	_ZBUS_ASSERT(sub->queue != NULL, "sub queue is required");
	_ZBUS_ASSERT(chan != NULL, "chan is required");

	return k_msgq_get(sub->queue, chan, timeout);
}