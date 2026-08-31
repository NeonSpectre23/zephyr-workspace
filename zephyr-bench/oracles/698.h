static inline int uhc_sof_enable(const struct device *dev)
{
	const struct uhc_api *api = dev->api;
	int ret;

	api->lock(dev);
	ret = api->sof_enable(dev);
	api->unlock(dev);

	return ret;
}