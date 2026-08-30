static inline int display_write(const struct device *dev, const uint16_t x,
				const uint16_t y,
				const struct display_buffer_descriptor *desc,
				const void *buf)
{
	struct display_driver_api *api =
		(struct display_driver_api *)dev->api;

	return api->write(dev, x, y, desc, buf);
}