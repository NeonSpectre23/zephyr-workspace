void cfb_framebuffer_deinit(const struct device *dev)
{
	struct char_framebuffer *fb = &char_fb;

	if (fb->buf) {
		k_free(fb->buf);
		fb->buf = NULL;
	}

}