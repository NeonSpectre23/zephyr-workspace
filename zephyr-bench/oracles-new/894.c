void sys_trace_k_thread_switched_in(void)
{
	struct k_thread *thread;

	if (k_is_pre_kernel()) {
		return;
	}

	thread = k_current_get();

	if (z_is_idle_thread_object(thread)) {
		SEGGER_SYSVIEW_OnIdle();
	} else {
		SEGGER_SYSVIEW_OnTaskStartExec((uint32_t)(uintptr_t)thread);
	}
}