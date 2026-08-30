int putc(int c, FILE *stream)
{
	return zephyr_fputc(c, stream);
}