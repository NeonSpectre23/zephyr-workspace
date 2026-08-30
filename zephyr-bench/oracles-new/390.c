int mtx_trylock(mtx_t *mutex)
{
	switch (pthread_mutex_trylock(mutex)) {
	case 0:
		return thrd_success;
	case EBUSY:
		return thrd_busy;
	default:
		return thrd_error;
	}
}