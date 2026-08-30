int ipc_service_deregister_endpoint(struct ipc_ept *ept)
{
	const struct ipc_service_backend *backend;
	int err;

	if (!ept) {
		LOG_ERR("Invalid endpoint");
		return -EINVAL;
	}

	if (!ept->instance) {
		LOG_ERR("Endpoint not registered\n");
		return -ENOENT;
	}

	backend = ept->instance->api;

	if (!backend || !backend->deregister_endpoint) {
		LOG_ERR("Invalid backend configuration");
		return -EIO;
	}

	err = backend->deregister_endpoint(ept->instance, ept->token);
	if (err != 0) {
		return err;
	}

	ept->instance = 0;

	return 0;
}