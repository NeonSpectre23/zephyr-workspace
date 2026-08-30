static inline int display_blanking_off(const struct device *dev)
{
	struct display_driver_api *api =
		(struct display_driver_api *)dev->api;

	if (api->blanking_off == NULL) {
		return -ENOSYS;
	}

	return api->blanking_off(dev);
}