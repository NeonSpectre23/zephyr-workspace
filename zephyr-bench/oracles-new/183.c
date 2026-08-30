int ipc_service_send(struct ipc_ept *ept, const void *data, size_t len)
{
	const struct ipc_service_backend *backend;

	if (!ept) {
		LOG_ERR("Invalid endpoint");
		return -EINVAL;
	}

	if (!ept->instance) {
		LOG_ERR("Endpoint not registered\n");
		return -ENOENT;
	}

	backend = ept->instance->api;

	if (!backend || !backend->send) {
		LOG_ERR("Invalid backend configuration");
		return -EIO;
	}

	return backend->send(ept->instance, ept->token, data, len);
}