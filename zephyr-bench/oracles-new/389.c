int mtx_timedlock(mtx_t *restrict mutex, const struct timespec *restrict time_point)
{
	switch (pthread_mutex_timedlock(mutex, time_point)) {
	case 0:
		return thrd_success;
	case ETIMEDOUT:
		return thrd_timedout;
	default:
		return thrd_error;
	}
}