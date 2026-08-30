osThreadId_t osMutexGetOwner(osMutexId_t mutex_id)
{
	struct cmsis_rtos_mutex_cb *mutex = (struct cmsis_rtos_mutex_cb *)mutex_id;

	if (k_is_in_isr() || (mutex == NULL)) {
		return NULL;
	}

	/* Mutex was not obtained before */
	if (mutex->z_mutex.lock_count == 0U) {
		return NULL;
	}

	return get_cmsis_thread_id(mutex->z_mutex.owner);
}