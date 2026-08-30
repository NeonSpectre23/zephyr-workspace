void sys_trace_k_thread_sched_set_priority(struct k_thread *thread, int prio)
{
	TRACING_STRING("%s: %p, priority: %d\n", __func__, thread, prio);
}