void mtx_destroy(mtx_t *mutex)
{
	(void)pthread_mutex_destroy(mutex);
}