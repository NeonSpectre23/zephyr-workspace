int snprintk(char *str, size_t size, const char *fmt, ...)
{
	va_list ap;
	int ret;

	va_start(ap, fmt);
	ret = vsnprintk(str, size, fmt, ap);
	va_end(ap);

	return ret;
}