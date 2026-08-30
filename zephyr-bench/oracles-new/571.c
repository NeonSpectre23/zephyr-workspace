uint32_t osThreadGetStackSpace(osThreadId_t thread_id)
{
	struct cmsis_rtos_thread_cb *tid = (struct cmsis_rtos_thread_cb *)thread_id;
	size_t unused;
	int ret;

	__ASSERT(tid, "");
	__ASSERT(is_cmsis_rtos_v2_thread(tid), "");
	__ASSERT(!k_is_in_isr(), "");

	ret = k_thread_stack_space_get(&tid->z_thread, &unused);
	if (ret != 0) {
		unused = 0;
	}

	return (uint32_t)unused;
}