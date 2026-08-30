char *asctime(const struct tm *tp)
{
	static char buf[DATE_STRING_BUF_SZ];

	return asctime_impl(tp, buf);
}