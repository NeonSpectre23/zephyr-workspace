osPriority osThreadGetPriority(osThreadId thread_id)
{
	k_tid_t thread = (k_tid_t)thread_id;
	uint32_t priority;

	if ((thread_id == NULL) || (k_is_in_isr())) {
		return osPriorityError;
	}

	priority = k_thread_priority_get(thread);
	return zephyr_to_cmsis_priority(priority);
}