int cnd_init(cnd_t *cond)
{
	switch (pthread_cond_init(cond, NULL)) {
	case 0:
		return thrd_success;
	case ENOMEM:
		return thrd_nomem;
	default:
		return thrd_error;
	}
}