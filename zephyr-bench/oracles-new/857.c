void sys_trace_k_condvar_signal_enter(struct k_condvar *condvar)
{
	ctf_top_condvar_signal_enter((uint32_t)(uintptr_t)condvar);
}