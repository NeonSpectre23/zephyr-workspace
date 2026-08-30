static inline int edac_parity_error_log_get(const struct device *dev,
					    uint64_t *value)
{
	const struct edac_driver_api *api =
		(const struct edac_driver_api *)dev->api;

	if (api->parity_error_log_get == NULL) {
		return -ENOSYS;
	}

	return api->parity_error_log_get(dev, value);
}