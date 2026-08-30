int hex2char(uint8_t x, char *c)
{
	if (x <= 9) {
		*c = x + (char)'0';
	} else  if (x <= 15) {
		*c = x - 10 + (char)'a';
	} else {
		return -EINVAL;
	}

	return 0;
}