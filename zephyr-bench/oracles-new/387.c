int mtx_init(mtx_t *mutex, int type)
{
	int ret;
	pthread_mutexattr_t attr;
	pthread_mutexattr_t *attrp = NULL;

	switch (type) {
	case mtx_plain:
	case mtx_timed:
		break;
	case mtx_plain | mtx_recursive:
	case mtx_timed | mtx_recursive:
		attrp = &attr;
		ret = pthread_mutexattr_init(attrp);
		__ASSERT_NO_MSG(ret == 0);

		ret = pthread_mutexattr_settype(attrp, PTHREAD_MUTEX_RECURSIVE);
		__ASSERT_NO_MSG(ret == 0);
		break;
	default:
		return thrd_error;
	}

	switch (pthread_mutex_init(mutex, attrp)) {
	case 0:
		ret = thrd_success;
		break;
	default:
		ret = thrd_error;
		break;
	}

	if (attrp != NULL) {
		(void)pthread_mutexattr_destroy(attrp);
	}

	return ret;
}