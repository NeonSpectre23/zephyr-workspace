void sys_trace_k_thread_wakeup(struct k_thread *thread)
{
	ctf_bounded_string_t name = {"unknown"};

	_get_thread_name(thread, &name);
	ctf_top_thread_wakeup((uint32_t)(uintptr_t)thread, name);
}