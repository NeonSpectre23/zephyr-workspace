int cfb_draw_text(const struct device *dev, const char *const str, int16_t x, int16_t y)
{
	return draw_text(dev, str, x, y, false);
}