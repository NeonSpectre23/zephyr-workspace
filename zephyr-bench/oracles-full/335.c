osStatus_t osThreadDetach(osThreadId_t thread_id)
{
	struct cmsis_rtos_thread_cb *tid = (struct cmsis_rtos_thread_cb *)thread_id;

	if ((tid == NULL) || (is_cmsis_rtos_v2_thread(tid) == NULL)) {
		return osErrorParameter;
	}

	if (k_is_in_isr()) {
		return osErrorISR;
	}

	if (_is_thread_cmsis_inactive(&tid->z_thread)) {
		return osErrorResource;
	}

	__ASSERT(tid->attr_bits != osThreadDetached,
		 "Thread already detached, behaviour undefined.");

	tid->attr_bits = osThreadDetached;

	return osOK;
}