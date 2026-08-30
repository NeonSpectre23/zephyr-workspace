void print_hex(const unsigned char *p, int len)
{
	while (len--) {
		printf("%02x", *p++);
	}
}