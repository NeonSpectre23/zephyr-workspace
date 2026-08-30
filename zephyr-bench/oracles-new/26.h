static inline int edac_errors_cor_get(const struct device *dev)
{
	const struct edac_driver_api *api =
		(const struct edac_driver_api *)dev->api;

	if (api->errors_cor_get == NULL) {
		return -ENOSYS;
	}

	return api->errors_cor_get(dev);
}