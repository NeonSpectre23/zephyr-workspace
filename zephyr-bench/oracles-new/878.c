void sys_trace_k_thread_join_exit(struct k_thread *thread, k_timeout_t timeout, int ret)
{
	ctf_top_thread_join_exit((uint32_t)(uintptr_t)thread, (uint32_t)timeout.ticks,
				 (int32_t)ret);
}