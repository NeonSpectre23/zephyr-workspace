static inline int edac_inject_set_error_type(const struct device *dev,
					     uint32_t error_type)
{
	const struct edac_driver_api *api =
		(const struct edac_driver_api *)dev->api;

	if (api->inject_set_error_type == NULL) {
		return -ENOSYS;
	}

	return api->inject_set_error_type(dev, error_type);
}