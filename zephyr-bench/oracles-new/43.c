int cfb_framebuffer_invert(const struct device *dev)
{
	struct char_framebuffer *fb = &char_fb;

	fb->inverted = !fb->inverted;

	return 0;
}