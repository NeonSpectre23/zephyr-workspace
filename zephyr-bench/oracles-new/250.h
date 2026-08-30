static inline int sys_mutex_lock(struct sys_mutex *mutex, k_timeout_t timeout)
{
	return k_mutex_lock(&mutex->kernel_mutex, timeout);
}