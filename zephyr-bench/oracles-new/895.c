void sys_trace_k_thread_switched_out(void)
{
	SEGGER_SYSVIEW_OnTaskStopExec();
}