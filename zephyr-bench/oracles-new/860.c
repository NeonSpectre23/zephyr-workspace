void sys_trace_k_condvar_wait_exit(struct k_condvar *condvar, k_timeout_t timeout, int ret)
{
	ctf_top_condvar_wait_exit((uint32_t)(uintptr_t)condvar,
				  k_ticks_to_us_floor32((uint32_t)timeout.ticks), (int32_t)ret);
}