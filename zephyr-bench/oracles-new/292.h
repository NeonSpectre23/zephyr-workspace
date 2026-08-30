static inline bool z_is_idle_thread_object(const struct k_thread *thread)
{
#ifdef CONFIG_MULTITHREADING
#ifdef CONFIG_SMP
	return thread->base.is_idle;
#else
	return thread == &z_idle_threads[0];
#endif /* CONFIG_SMP */
#else
	return false;
#endif /* CONFIG_MULTITHREADING */
}