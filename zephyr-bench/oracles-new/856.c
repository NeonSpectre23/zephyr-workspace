void sys_trace_k_condvar_signal_blocking(struct k_condvar *condvar, k_timeout_t timeout)
{
	ctf_top_condvar_signal_blocking((uint32_t)(uintptr_t)condvar,
					k_ticks_to_us_floor32((uint32_t)timeout.ticks));
}