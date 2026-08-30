int thrd_create(thrd_t *thr, thrd_start_t func, void *arg)
{
	typedef void *(*pthread_func_t)(void *arg);

	pthread_func_t pfunc = (pthread_func_t)func;

	switch (pthread_create(thr, NULL, pfunc, arg)) {
	case 0:
		return thrd_success;
	case EAGAIN:
		return thrd_nomem;
	default:
		return thrd_error;
	}
}