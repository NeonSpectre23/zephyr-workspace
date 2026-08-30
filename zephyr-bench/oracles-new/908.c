int thrd_sleep(const struct timespec *duration, struct timespec *remaining)
{
	if (sys_clock_nanosleep(SYS_CLOCK_REALTIME, 0, duration, remaining) != 0) {
		return thrd_error;
	}

	return thrd_success;
}