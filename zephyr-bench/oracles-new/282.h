static inline k_timeout_t timespec_to_timeout(const struct timespec *req, struct timespec *rem)
{
	k_timeout_t timeout;
	struct timespec temp = SYS_TIMESPEC_NO_WAIT;

	__ASSERT_NO_MSG((req != NULL) && timespec_is_valid(req));

	if (timespec_compare(req, &temp) <= 0) {
		if (rem != NULL) {
			*rem = *req;
		}
		/* equivalent of K_NO_WAIT without including kernel.h */
		timeout.ticks = 0;
		return timeout;
	}

	temp = SYS_TIMESPEC_FOREVER;

	if (timespec_compare(req, &temp) == 0) {
		if (rem != NULL) {
			*rem = SYS_TIMESPEC_NO_WAIT;
		}
		/* equivalent of K_FOREVER without including kernel.h */
		timeout.ticks = K_TICKS_FOREVER;
		return timeout;
	}

	temp = SYS_TIMESPEC_MAX;

	if (timespec_compare(req, &temp) >= 0) {
		/* round down to align to max ticks */
		timeout.ticks = K_TICK_MAX;
	} else {
		/* round up to align to next tick boundary */
		timeout.ticks = CLAMP(k_ns_to_ticks_ceil64(req->tv_nsec) +
					      k_sec_to_ticks_ceil64(req->tv_sec),
				      K_TICK_MIN, K_TICK_MAX);
	}

	if (rem != NULL) {
		timespec_from_timeout(timeout, rem);
		timespec_sub(rem, req);
		timespec_negate(rem);
	}

	return timeout;
}