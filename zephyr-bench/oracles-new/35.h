static inline int edac_notify_callback_set(const struct device *dev,
					   edac_notify_callback_f cb)
{
	const struct edac_driver_api *api = (const struct edac_driver_api *)dev->api;

	if (api->notify_cb_set == NULL) {
		return -ENOSYS;
	}

	return api->notify_cb_set(dev, cb);
}