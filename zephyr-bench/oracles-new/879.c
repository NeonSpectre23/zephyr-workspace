void sys_trace_k_thread_name_set(struct k_thread *thread, int ret)
{
	ctf_bounded_string_t name = {"unknown"};

	_get_thread_name(thread, &name);
	ctf_top_thread_name_set((uint32_t)(uintptr_t)thread, name);
}