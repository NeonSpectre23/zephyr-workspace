void sys_trace_k_condvar_init(struct k_condvar *condvar, int ret)
{
	ctf_top_condvar_init((uint32_t)(uintptr_t)condvar, (int32_t)ret);
}