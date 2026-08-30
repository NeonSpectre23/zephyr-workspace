void zbus_proxy_agent_listener_cb(const struct zbus_channel *chan,
				  const struct zbus_proxy_agent *agent)
{
	int ret;
	const void *msg;
	const char *chan_name;
	size_t chan_name_len;
	struct zbus_proxy_msg tx_msg = {0};

	if (chan->message_size > CONFIG_ZBUS_PROXY_AGENT_MAX_MESSAGE_SIZE) {
		LOG_ERR("Message size %zu exceeds maximum %d in proxy agent listener callback",
			chan->message_size, CONFIG_ZBUS_PROXY_AGENT_MAX_MESSAGE_SIZE);
		return;
	}

	LOG_DBG("Received message on channel '%s' for proxy agent '%s'", chan->name, agent->name);

	msg = zbus_chan_const_msg(chan);
	chan_name = _ZBUS_CHAN_NAME(chan);
	chan_name_len = strlen(chan_name) + 1; /* includes NUL terminator */

	if (chan_name_len > sizeof(tx_msg.channel_name)) {
		LOG_ERR("Channel name '%s' too long for proxy transport (%zu > %zu)", chan_name,
			chan_name_len, sizeof(tx_msg.channel_name));
		return;
	}

	tx_msg.message_size = chan->message_size;
	memcpy(tx_msg.message, msg, chan->message_size);
	tx_msg.channel_name_len = chan_name_len;
	memcpy(tx_msg.channel_name, chan_name, chan_name_len);

	ret = agent->backend_api->backend_send(agent, &tx_msg);
	if (ret < 0) {
		LOG_ERR("Failed to send message via proxy agent '%s' backend: %d", agent->name,
			ret);
	}
}