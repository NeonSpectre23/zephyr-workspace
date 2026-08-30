void sys_trace_k_mutex_lock_enter(struct k_mutex *mutex, k_timeout_t timeout)
{
	ctf_top_mutex_lock_enter((uint32_t)(uintptr_t)mutex,
				 k_ticks_to_us_floor32((uint32_t)timeout.ticks));
}