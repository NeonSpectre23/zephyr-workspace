int cfb_framebuffer_init(const struct device *dev)
{
	const struct display_driver_api *api = dev->api;
	struct char_framebuffer *fb = &char_fb;
	struct display_capabilities cfg;

	__ASSERT_NO_MSG(DEVICE_API_IS(display, dev));

	api->get_capabilities(dev, &cfg);

	STRUCT_SECTION_COUNT(cfb_font, &fb->numof_fonts);

	LOG_DBG("number of fonts %d", fb->numof_fonts);

	fb->x_res = cfg.x_resolution;
	fb->y_res = cfg.y_resolution;
	fb->ppt = 8U;
	fb->pixel_format = cfg.current_pixel_format;
	fb->screen_info = cfg.screen_info;
	fb->kerning = 0;
	fb->inverted = false;

	fb->fonts = TYPE_SECTION_START(cfb_font);
	fb->font_idx = 0U;

	fb->size = fb->x_res * fb->y_res / fb->ppt;
	fb->buf = k_malloc(fb->size);
	if (!fb->buf) {
		return -ENOMEM;
	}

	memset(fb->buf, 0, fb->size);

	return 0;
}