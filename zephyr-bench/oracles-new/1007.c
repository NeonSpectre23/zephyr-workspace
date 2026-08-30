int vfprintf(FILE *ZRESTRICT stream, const char *ZRESTRICT format,
	     va_list vargs)
{
	int r;

	r = cbvprintf(fputc, DESC(stream), format, vargs);

	return r;
}