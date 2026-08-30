int vprintf(const char *ZRESTRICT format, va_list vargs)
{
	int r;

	r = cbvprintf(fputc, DESC(stdout), format, vargs);

	return r;
}