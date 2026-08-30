__attribute_const__
static inline k_tid_t k_current_get(void)
{
	__ASSERT(!k_is_pre_kernel(), "k_current_get called pre-kernel");

#ifdef CONFIG_CURRENT_THREAD_USE_TLS

	/* Thread-local cache of current thread ID, set in z_thread_entry() */
	extern Z_THREAD_LOCAL k_tid_t z_tls_current;

	return z_tls_current;
#else
	return k_sched_current_thread_query();
#endif
}