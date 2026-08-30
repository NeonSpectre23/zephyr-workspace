int cfb_set_kerning(const struct device *dev, int8_t kerning)
{
	char_fb.kerning = kerning;

	return 0;
}