int cnd_broadcast(cnd_t *cond)
{
	switch (pthread_cond_broadcast(cond)) {
	case 0:
		return thrd_success;
	default:
		return thrd_error;
	}
}