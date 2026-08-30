static inline int edac_errors_uc_get(const struct device *dev)
{
	const struct edac_driver_api *api =
		(const struct edac_driver_api *)dev->api;

	if (api->errors_uc_get == NULL) {
		return -ENOSYS;
	}

	return api->errors_uc_get(dev);
}