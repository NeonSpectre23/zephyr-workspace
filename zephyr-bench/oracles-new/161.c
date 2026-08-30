struct tm *gmtime_r(const time_t *ZRESTRICT timep,
		    struct tm *ZRESTRICT result)
{
	time_t z = *timep;
	bigint_type days = (z >= 0 ? z : z - 86399) / 86400;
	unsigned int rem = z - days * 86400;

	*result = (struct tm){ 0 };

	time_civil_from_days(days, result);

	result->tm_hour = rem / 60U / 60U;
	rem -= result->tm_hour * 60 * 60;
	result->tm_min = rem / 60;
	result->tm_sec = rem - result->tm_min * 60;

	return result;
}