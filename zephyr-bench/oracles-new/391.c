int mtx_unlock(mtx_t *mutex)
{
	switch (pthread_mutex_unlock(mutex)) {
	case 0:
		return thrd_success;
	default:
		return thrd_error;
	}
}