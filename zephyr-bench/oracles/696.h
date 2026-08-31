static inline int uhc_bus_reset(const struct device *dev)
{
	const struct uhc_api *api = dev->api;
	int ret;

	api->lock(dev);
	ret = api->bus_reset(dev);
	api->unlock(dev);

	return ret;
}