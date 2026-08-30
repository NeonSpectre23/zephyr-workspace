int zbus_init_proxy_agent(const struct zbus_proxy_agent *agent)
{
	_ZBUS_ASSERT(agent != NULL, "Proxy agent configuration is NULL in init");
	_ZBUS_ASSERT(agent->thread_id != NULL, "Thread ID storage is NULL in proxy agent init");
	_ZBUS_ASSERT(agent->backend_api != NULL, "Backend API is NULL in proxy agent init");
	_ZBUS_ASSERT(agent->thread != NULL, "Thread is NULL in proxy agent init");
	_ZBUS_ASSERT(agent->msgq != NULL, "Message queue is NULL in proxy agent init");
	_ZBUS_ASSERT(agent->stack != NULL, "Thread stack is NULL in proxy agent init");

	int ret;

	*agent->thread_id = k_thread_create(
		agent->thread, agent->stack, CONFIG_ZBUS_PROXY_AGENT_WORK_QUEUE_STACK_SIZE,
		proxy_agent_thread_fn, (void *)agent, NULL, NULL,
		CONFIG_ZBUS_PROXY_AGENT_WORK_QUEUE_PRIORITY, 0, K_NO_WAIT);
	k_thread_name_set(*agent->thread_id, agent->name);

	ret = agent->backend_api->backend_set_recv_cb(agent, zbus_proxy_agent_receive_cb);
	if (ret < 0) {
		LOG_ERR("Failed to set receive callback for proxy agent %s: %d", agent->name, ret);
		k_thread_abort(*agent->thread_id);
		*agent->thread_id = NULL;
		return ret;
	}

	ret = agent->backend_api->backend_init(agent);
	if (ret < 0) {
		LOG_ERR("Failed to initialize backend for proxy agent %s: %d", agent->name, ret);
		k_thread_abort(*agent->thread_id);
		*agent->thread_id = NULL;
		return ret;
	}

	LOG_DBG("Proxy agent %s initialized successfully", agent->name);

	return 0;
}