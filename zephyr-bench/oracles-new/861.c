void sys_trace_k_mutex_init(struct k_mutex *mutex, int ret)
{
	ctf_top_mutex_init((uint32_t)(uintptr_t)mutex, (int32_t)ret);
}