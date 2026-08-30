int cbprintf_package(void *packaged, size_t len, uint32_t flags,
		     const char *format, ...)
{
	va_list ap;
	int ret;

	va_start(ap, format);
	ret = cbvprintf_package(packaged, len, flags, format, ap);
	va_end(ap);
	return ret;
}