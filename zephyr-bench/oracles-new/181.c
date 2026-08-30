int ipc_service_open_instance(const struct device *instance)
{
	const struct ipc_service_backend *backend;

	if (!instance) {
		LOG_ERR("Invalid instance");
		return -EINVAL;
	}

	backend = (const struct ipc_service_backend *) instance->api;

	if (!backend) {
		LOG_ERR("Invalid backend configuration");
		return -EIO;
	}

	if (!backend->open_instance) {
		/* maybe not needed on backend */
		return 0;
	}

	return backend->open_instance(instance);
}