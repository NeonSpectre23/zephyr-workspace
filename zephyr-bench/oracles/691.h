static inline void timespec_from_timeout(k_timeout_t timeout, struct timespec *ts)
{
	__ASSERT_NO_MSG(ts != NULL);
	__ASSERT_NO_MSG(Z_IS_TIMEOUT_RELATIVE(timeout) ||
			(IS_ENABLED(CONFIG_TIMEOUT_64BIT) &&
			 K_TIMEOUT_EQ(timeout, (k_timeout_t){K_TICKS_FOREVER})));

	/* equivalent of K_FOREVER without including kernel.h */
	if (K_TIMEOUT_EQ(timeout, (k_timeout_t){K_TICKS_FOREVER})) {
		/* duration == K_TICKS_FOREVER ticks */
		*ts = SYS_TIMESPEC_FOREVER;
		/* equivalent of K_NO_WAIT without including kernel.h */
	} else if (K_TIMEOUT_EQ(timeout, (k_timeout_t){0})) {
		/* duration <= 0 ticks */
		*ts = SYS_TIMESPEC_NO_WAIT;
	} else {
		*ts = SYS_TICKS_TO_TIMESPEC(timeout.ticks);
	}

	__ASSERT_NO_MSG(timespec_is_valid(ts));
}