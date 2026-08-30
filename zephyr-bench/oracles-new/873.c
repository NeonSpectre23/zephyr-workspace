void sys_trace_k_thread_abort_enter(struct k_thread *thread)
{
	TRACING_STRING("%s: %p\n", __func__, thread);
}