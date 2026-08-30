int cfb_framebuffer_clear(const struct device *dev, bool clear_display)
{
	const struct char_framebuffer *fb = &char_fb;

	if (!fb->buf) {
		return -ENODEV;
	}

	memset(fb->buf, 0, fb->size);

	if (clear_display) {
		cfb_framebuffer_finalize(dev);
	}

	return 0;
}