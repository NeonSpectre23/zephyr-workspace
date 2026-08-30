int cfb_framebuffer_finalize(const struct device *dev)
{
	const struct display_driver_api *api = dev->api;
	const struct char_framebuffer *fb = &char_fb;
	int err;

	__ASSERT_NO_MSG(DEVICE_API_IS(display, dev));

	if (!fb->buf) {
		return -ENODEV;
	}

	struct display_buffer_descriptor desc = {
		.buf_size = fb->size,
		.width = fb->x_res,
		.height = fb->y_res,
		.pitch = fb->x_res,
	};

	if ((fb->pixel_format == PIXEL_FORMAT_MONO10) != fb->inverted) {
		cfb_invert(fb);
		err = api->write(dev, 0, 0, &desc, fb->buf);
		cfb_invert(fb);
		return err;
	}

	return api->write(dev, 0, 0, &desc, fb->buf);
}