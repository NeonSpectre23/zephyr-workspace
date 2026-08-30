void sys_trace_k_thread_sleep_exit(k_timeout_t timeout, int ret)
{
	ctf_top_thread_sleep_exit(k_ticks_to_us_floor32((uint32_t)timeout.ticks), (uint32_t)ret);
}