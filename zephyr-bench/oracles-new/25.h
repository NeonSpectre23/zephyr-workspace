static inline int edac_ecc_error_log_get(const struct device *dev,
					 uint64_t *value)
{
	const struct edac_driver_api *api =
		(const struct edac_driver_api *)dev->api;

	if (api->ecc_error_log_get == NULL) {
		return -ENOSYS;
	}

	return api->ecc_error_log_get(dev, value);
}