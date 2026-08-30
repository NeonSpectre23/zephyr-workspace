int char2hex(char c, uint8_t *x)
{
	if ((c >= '0') && (c <= '9')) {
		*x = c - '0';
	} else if ((c >= 'a') && (c <= 'f')) {
		*x = c - 'a' + 10;
	} else if ((c >= 'A') && (c <= 'F')) {
		*x = c - 'A' + 10;
	} else {
		return -EINVAL;
	}

	return 0;
}