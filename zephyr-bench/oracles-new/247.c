struct tm *localtime(const time_t *timer)
{
	return gmtime(timer);
}