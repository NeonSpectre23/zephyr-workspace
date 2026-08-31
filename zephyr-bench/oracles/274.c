uint32_t osThreadFlagsGet(void)
{
	struct cmsis_rtos_thread_cb *tid;

	if (k_is_in_isr()) {
		return 0;
	}

	tid = (struct cmsis_rtos_thread_cb *)osThreadGetId();
	if (tid == NULL) {
		return 0;
	} else {
		return tid->signal_results;
	}
}