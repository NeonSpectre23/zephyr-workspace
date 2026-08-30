int sys_clock_gettime(int clock_id, struct timespec *ts)
{
	if (!is_valid_clock_id(clock_id)) {
		return -EINVAL;
	}

	switch (clock_id) {
	case SYS_CLOCK_REALTIME: {
		struct timespec offset;

		timespec_from_ticks(k_uptime_ticks(), ts);
		sys_clock_getrtoffset(&offset);
		if (unlikely(!timespec_add(ts, &offset))) {
			/* Saturate rather than reporting an overflow in 292 billion years */
			*ts = (struct timespec){
				.tv_sec = (time_t)INT64_MAX,
				.tv_nsec = NSEC_PER_SEC - 1,
			};
		}
	} break;

	case SYS_CLOCK_MONOTONIC:
		timespec_from_ticks(k_uptime_ticks(), ts);
		break;

	default:
		CODE_UNREACHABLE;
		return -EINVAL; /* Should never reach here */
	}

	__ASSERT_NO_MSG(timespec_is_valid(ts));

	return 0;
}