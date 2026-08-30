void sys_trace_k_thread_join_blocking(struct k_thread *thread, k_timeout_t timeout)
{
	ctf_top_thread_join_blocking((uint32_t)(uintptr_t)thread, (uint32_t)timeout.ticks);
}