osStatus_t osThreadJoin(osThreadId_t thread_id)
{
	struct cmsis_rtos_thread_cb *tid = (struct cmsis_rtos_thread_cb *)thread_id;
	int ret = 0;

	if ((tid == NULL) || (is_cmsis_rtos_v2_thread(tid) == NULL)) {
		return osErrorParameter;
	}

	if (k_is_in_isr()) {
		return osErrorISR;
	}

	if (_is_thread_cmsis_inactive(&tid->z_thread)) {
		return osErrorResource;
	}

	if (tid->attr_bits != osThreadJoinable) {
		return osErrorResource;
	}

	ret = k_thread_join(&tid->z_thread, K_FOREVER);
	return (ret == 0) ? osOK : osErrorResource;
}