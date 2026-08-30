int cnd_signal(cnd_t *cond)
{
	switch (pthread_cond_signal(cond)) {
	case 0:
		return thrd_success;
	case ENOMEM:
		return thrd_nomem;
	default:
		return thrd_error;
	}
}