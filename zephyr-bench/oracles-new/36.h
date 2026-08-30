static inline int edac_parity_error_log_clear(const struct device *dev)
{
	const struct edac_driver_api *api =
		(const struct edac_driver_api *)dev->api;

	if (api->parity_error_log_clear == NULL) {
		return -ENOSYS;
	}

	return api->parity_error_log_clear(dev);
}