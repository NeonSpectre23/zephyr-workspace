uint32_t osThreadGetStackSize(osThreadId_t thread_id)
{
	struct cmsis_rtos_thread_cb *tid = (struct cmsis_rtos_thread_cb *)thread_id;

	__ASSERT(tid, "");
	__ASSERT(is_cmsis_rtos_v2_thread(tid), "");
	__ASSERT(!k_is_in_isr(), "");

	return tid->z_thread.stack_info.size;
}