void sys_trace_k_thread_abort_exit(struct k_thread *thread)
{
	TRACING_STRING("%s: %p\n", __func__, thread);
}