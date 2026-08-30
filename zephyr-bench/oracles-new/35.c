int cfb_draw_line(const struct device *dev, const struct cfb_position *start,
		  const struct cfb_position *end)
{
	struct char_framebuffer *fb = &char_fb;

	draw_line(fb, start->x, start->y, end->x, end->y);

	return 0;
}