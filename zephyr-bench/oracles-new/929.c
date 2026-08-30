int tss_create(tss_t *key, tss_dtor_t destructor)
{
	switch (pthread_key_create(key, destructor)) {
	case 0:
		return thrd_success;
	case EAGAIN:
		return thrd_busy;
	case ENOMEM:
		return thrd_nomem;
	default:
		return thrd_error;
	}
}