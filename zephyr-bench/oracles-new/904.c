int thrd_detach(thrd_t thr)
{
	switch (pthread_detach(thr)) {
	case 0:
		return thrd_success;
	default:
		return thrd_error;
	}
}