char *asctime_r(const struct tm *tp, char *buf)
{
	return asctime_impl(tp, buf);
}