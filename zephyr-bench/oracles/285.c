osStatus osThreadTerminate(osThreadId thread_id)
{
	if (thread_id == NULL) {
		return osErrorParameter;
	}

	if (k_is_in_isr()) {
		return osErrorISR;
	}

	if (_is_thread_cmsis_inactive((k_tid_t)thread_id)) {
		return osErrorResource;
	}

	k_thread_abort((k_tid_t)thread_id);
	return osOK;
}