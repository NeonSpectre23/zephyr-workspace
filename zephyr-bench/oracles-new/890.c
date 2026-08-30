void sys_trace_k_thread_sleep_enter(k_timeout_t timeout)
{
	ctf_top_thread_sleep_enter(k_ticks_to_us_floor32((uint32_t)timeout.ticks));
}