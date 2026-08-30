int thrd_join(thrd_t thr, int *res)
{
	void *ret;

	switch (pthread_join(thr, &ret)) {
	case 0:
		if (res != NULL) {
			*res = POINTER_TO_INT(ret);
		}
		return thrd_success;
	default:
		return thrd_error;
	}
}