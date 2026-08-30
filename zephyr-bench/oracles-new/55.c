void cnd_destroy(cnd_t *cond)
{
	(void)pthread_cond_destroy(cond);
}