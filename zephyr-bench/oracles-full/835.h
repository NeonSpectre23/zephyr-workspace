static inline int sys_mutex_unlock(struct sys_mutex *mutex)
{
	return k_mutex_unlock(&mutex->kernel_mutex);
}