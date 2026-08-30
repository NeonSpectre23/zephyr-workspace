static inline void k_thread_start(k_tid_t thread)
{
	k_wakeup(thread);
}