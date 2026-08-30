struct tm *localtime_r(const time_t *timer, struct tm *result)
{
	return gmtime_r(timer, result);
}