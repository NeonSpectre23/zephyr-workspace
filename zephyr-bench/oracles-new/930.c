void tss_delete(tss_t key)
{
	(void)pthread_key_delete(key);
}