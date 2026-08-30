void sys_trace_k_mutex_unlock_exit(struct k_mutex *mutex, int ret)
{
	ctf_top_mutex_unlock_exit((uint32_t)(uintptr_t)mutex, (int32_t)ret);
}