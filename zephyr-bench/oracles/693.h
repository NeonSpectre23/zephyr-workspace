static inline bool timespec_negate(struct timespec *ts)
{
	__ASSERT_NO_MSG((ts != NULL) && timespec_is_valid(ts));

#if defined(CONFIG_SPEED_OPTIMIZATIONS) && HAS_BUILTIN(__builtin_sub_overflow)

	return !__builtin_sub_overflow(0LL, ts->tv_sec, &ts->tv_sec) &&
	       !__builtin_sub_overflow(0L, ts->tv_nsec, &ts->tv_nsec) && timespec_normalize(ts);

#else

	if (ts->tv_sec == SYS_TIME_T_MIN) {
		/* -SYS_TIME_T_MIN > SYS_TIME_T_MAX, so positive integer overflow would occur */
		return false;
	}

	ts->tv_sec = -ts->tv_sec;
	ts->tv_nsec = -ts->tv_nsec;

	return timespec_normalize(ts);

#endif
}