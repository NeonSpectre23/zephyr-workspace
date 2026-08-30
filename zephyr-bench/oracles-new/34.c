int cfb_draw_circle(const struct device *dev, const struct cfb_position *center, uint16_t radius)
{
	struct char_framebuffer *fb = &char_fb;
	uint16_t x = 0;
	int16_t y = -radius;
	int16_t p = -radius;

	/* Using the Midpoint Circle Algorithm */
	while (x < -y) {
		if (p > 0) {
			p += 2 * (x + ++y) + 1;
		} else {
			p += 2 * x + 1;
		}

		draw_point(fb, center->x + x, center->y + y);
		draw_point(fb, center->x - x, center->y + y);
		draw_point(fb, center->x + x, center->y - y);
		draw_point(fb, center->x - x, center->y - y);
		draw_point(fb, center->x + y, center->y + x);
		draw_point(fb, center->x + y, center->y - x);
		draw_point(fb, center->x - y, center->y + x);
		draw_point(fb, center->x - y, center->y - x);

		x++;
	}

	return 0;
}