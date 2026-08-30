int ipc_service_register_endpoint(const struct device *instance,
				  struct ipc_ept *ept,
				  const struct ipc_ept_cfg *cfg)
{
	const struct ipc_service_backend *backend;

	if (!instance || !ept || !cfg) {
		LOG_ERR("Invalid instance, endpoint or configuration");
		return -EINVAL;
	}

	backend = (const struct ipc_service_backend *) instance->api;

	if (!backend || !backend->register_endpoint) {
		LOG_ERR("Invalid backend configuration");
		return -EIO;
	}

	LOG_DBG("Register endpoint %s", cfg->name ? cfg->name : "");

	ept->instance = instance;

	return backend->register_endpoint(instance, &ept->token, cfg);
}