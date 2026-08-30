int cfb_framebuffer_set_font(const struct device *dev, uint8_t idx)
{
	struct char_framebuffer *fb = &char_fb;

	if (idx >= fb->numof_fonts) {
		return -EINVAL;
	}

	fb->font_idx = idx;

	return 0;
}