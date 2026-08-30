static inline int
display_set_pixel_format(const struct device *dev,
			 const enum display_pixel_format pixel_format)
{
	struct display_driver_api *api =
		(struct display_driver_api *)dev->api;

	if (api->set_pixel_format == NULL) {
		return -ENOSYS;
	}

	return api->set_pixel_format(dev, pixel_format);
}