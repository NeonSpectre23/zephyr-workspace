bool timespec_normalize(struct timespec *ts)
{
	__ASSERT_NO_MSG(ts != NULL);

	long sec;

	if (ts->tv_nsec >= (long)NSEC_PER_SEC) {
		sec = ts->tv_nsec / (long)NSEC_PER_SEC;
	} else if (ts->tv_nsec < 0) {
		sec = DIV_ROUND_UP((unsigned long)-ts->tv_nsec, NSEC_PER_SEC);
	} else {
		sec = 0;
	}

	if ((ts->tv_nsec < 0) && (ts->tv_sec < 0) && (ts->tv_sec - SYS_TIME_T_MIN < sec)) {
		/*
		 * When `tv_nsec` is negative and `tv_sec` is already most negative,
		 * further subtraction would cause integer overflow.
		 */
		return false;
	}

	if ((ts->tv_nsec >= (long)NSEC_PER_SEC) && (ts->tv_sec > 0) &&
	    (SYS_TIME_T_MAX - ts->tv_sec < sec)) {
		/*
		 * When `tv_nsec` is >= `NSEC_PER_SEC` and `tv_sec` is already most
		 * positive, further addition would cause integer overflow.
		 */
		return false;
	}

	if (ts->tv_nsec >= (long)NSEC_PER_SEC) {
		ts->tv_sec += sec;
		ts->tv_nsec -= sec * (long)NSEC_PER_SEC;
	} else if (ts->tv_nsec < 0) {
		ts->tv_sec -= sec;
		ts->tv_nsec += sec * (long)NSEC_PER_SEC;
	} else {
		/* no change: SonarQube was complaining */
	}

	__ASSERT_NO_MSG(timespec_is_valid(ts));

	return true;
}