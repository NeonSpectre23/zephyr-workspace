time_t timeutil_timegm(const struct tm *tm)
{
	int64_t time = timeutil_timegm64(tm);
	time_t rv = (time_t)time;

	errno = 0;
	if ((sizeof(rv) == sizeof(int32_t))
	    && ((time < (int64_t)INT32_MIN)
		|| (time > (int64_t)INT32_MAX))) {
		errno = ERANGE;
		rv = -1;
	}

	return rv;
}