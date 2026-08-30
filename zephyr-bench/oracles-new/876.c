void sys_trace_k_thread_info(struct k_thread *thread)
{
#if defined(CONFIG_THREAD_STACK_INFO)
	ctf_bounded_string_t name = {"unknown"};

	_get_thread_name(thread, &name);
	ctf_top_thread_info((uint32_t)(uintptr_t)thread, name, thread->stack_info.start,
			    thread->stack_info.size);
#endif
}