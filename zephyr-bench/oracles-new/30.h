static inline int edac_inject_get_param1(const struct device *dev,
					 uint64_t *value)
{
	const struct edac_driver_api *api =
		(const struct edac_driver_api *)dev->api;

	if (api->inject_get_param1 == NULL) {
		return -ENOSYS;
	}

	return api->inject_get_param1(dev, value);

}