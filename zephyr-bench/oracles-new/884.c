void sys_trace_k_thread_sched_abort(struct k_thread *thread)
{
	ctf_bounded_string_t name = {"unknown"};

	_get_thread_name(thread, &name);
	ctf_top_thread_sched_abort((uint32_t)(uintptr_t)thread, name);
}