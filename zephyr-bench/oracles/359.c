int snprintfcb(char *str, size_t size, const char *format, ...)
{
	va_list ap;
	int rc;

	va_start(ap, format);
	rc = vsnprintfcb(str, size, format, ap);
	va_end(ap);

	return rc;
}