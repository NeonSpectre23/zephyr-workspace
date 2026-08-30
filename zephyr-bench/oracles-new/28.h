static inline int edac_inject_error_trigger(const struct device *dev)
{
	const struct edac_driver_api *api =
		(const struct edac_driver_api *)dev->api;

	if (api->inject_error_trigger == NULL) {
		return -ENOSYS;
	}

	return api->inject_error_trigger(dev);
}