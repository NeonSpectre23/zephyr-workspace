void sys_trace_k_thread_create(struct k_thread *thread, size_t stack_size, int prio)
{
	ctf_bounded_string_t name = {"unknown"};

	_get_thread_name(thread, &name);
	ctf_top_thread_create((uint32_t)(uintptr_t)thread, thread->base.prio, name);

#if defined(CONFIG_THREAD_STACK_INFO)
	ctf_top_thread_info((uint32_t)(uintptr_t)thread, name, thread->stack_info.start,
			    thread->stack_info.size);
#endif
}