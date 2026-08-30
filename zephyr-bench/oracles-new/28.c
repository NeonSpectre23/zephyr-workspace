void call_once(once_flag *flag, void (*func)(void))
{
	(void)pthread_once((pthread_once_t *)flag, func);
}