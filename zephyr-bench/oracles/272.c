__NO_RETURN void osThreadExit(void)
{
	struct cmsis_rtos_thread_cb *tid;

	__ASSERT(!k_is_in_isr(), "");
	tid = osThreadGetId();

	k_thread_abort((k_tid_t)&tid->z_thread);

	CODE_UNREACHABLE;
}