int zbus_sub_wait_msg(const struct zbus_observer *sub, const struct zbus_channel **chan, void *msg,
		      k_timeout_t timeout)
{
	_ZBUS_ASSERT(!k_is_in_isr(), "zbus_sub_wait_msg cannot be used inside ISRs");
	_ZBUS_ASSERT(sub != NULL, "sub is required");
	_ZBUS_ASSERT(sub->type == ZBUS_OBSERVER_MSG_SUBSCRIBER_TYPE,
		     "sub must be a MSG_SUBSCRIBER");
	_ZBUS_ASSERT(sub->message_fifo != NULL, "sub message_fifo is required");
	_ZBUS_ASSERT(chan != NULL, "chan is required");
	_ZBUS_ASSERT(msg != NULL, "msg is required");

	struct net_buf *buf = k_fifo_get(sub->message_fifo, timeout);

	if (buf == NULL) {
		return -ENOMSG;
	}

	*chan = *((struct zbus_channel **)net_buf_user_data(buf));

	memcpy(msg, net_buf_remove_mem(buf, zbus_chan_msg_size(*chan)), zbus_chan_msg_size(*chan));

	net_buf_unref(buf);

	return 0;
}