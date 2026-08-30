int cfb_draw_rect(const struct device *dev, const struct cfb_position *start,
		  const struct cfb_position *end)
{
	struct char_framebuffer *fb = &char_fb;

	draw_line(fb, start->x, start->y, end->x, start->y);
	draw_line(fb, end->x, start->y, end->x, end->y);
	draw_line(fb, end->x, end->y, start->x, end->y);
	draw_line(fb, start->x, end->y, start->x, start->y);

	return 0;
}