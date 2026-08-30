static inline int edac_ecc_error_log_clear(const struct device *dev)
{
	const struct edac_driver_api *api =
		(const struct edac_driver_api *)dev->api;

	if (api->ecc_error_log_clear == NULL) {
		return -ENOSYS;
	}

	return api->ecc_error_log_clear(dev);
}