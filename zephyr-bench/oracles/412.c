int64_t timeutil_timegm64(const struct tm *tm)
{
	int64_t y = TIME_UTILS_BASE_YEAR + (int64_t)tm->tm_year;
	unsigned int m = tm->tm_mon + 1;
	unsigned int d = tm->tm_mday - 1;
	int64_t ndays = time_days_from_civil(y, m, d);
	int64_t time = tm->tm_sec;

	time += 60LL * (tm->tm_min + 60LL * tm->tm_hour);
	time += 86400LL * ndays;

	return time;
}