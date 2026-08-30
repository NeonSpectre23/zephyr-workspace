int mtx_lock(mtx_t *mutex)
{
	switch (pthread_mutex_lock(mutex)) {
	case 0:
		return thrd_success;
	default:
		return thrd_error;
	}
}