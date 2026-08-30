int cnd_wait(cnd_t *cond, mtx_t *mtx)
{
	switch (pthread_cond_wait(cond, mtx)) {
	case 0:
		return thrd_success;
	default:
		return thrd_error;
	}
}