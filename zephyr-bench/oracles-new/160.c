struct tm *gmtime(const time_t *timep)
{
	return gmtime_r(timep, &gmtime_result);
}