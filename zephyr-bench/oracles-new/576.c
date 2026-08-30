osStatus osThreadSetPriority(osThreadId thread_id, osPriority priority)
{
	if (thread_id == NULL) {
		return osErrorParameter;
	}

	if (k_is_in_isr()) {
		return osErrorISR;
	}

	if (priority < osPriorityIdle || priority > osPriorityRealtime) {
		return osErrorValue;
	}

	if (_is_thread_cmsis_inactive((k_tid_t)thread_id)) {
		return osErrorResource;
	}

	k_thread_priority_set((k_tid_t)thread_id,
				cmsis_to_zephyr_priority(priority));

	return osOK;
}