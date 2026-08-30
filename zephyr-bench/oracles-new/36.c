int cfb_draw_point(const struct device *dev, const struct cfb_position *pos)
{
	struct char_framebuffer *fb = &char_fb;

	draw_point(fb, pos->x, pos->y);

	return 0;
}