int32_t osSemaphoreWait(osSemaphoreId semaphore_id, uint32_t timeout)
{
	struct k_sem *semaphore = (struct k_sem *) semaphore_id;
	int status;

	if (semaphore_id == NULL) {
		return -1;
	}

	if (k_is_in_isr()) {
		return -1;
	}

	if (timeout == osWaitForever) {
		status = k_sem_take(semaphore, K_FOREVER);
	} else if (timeout == 0U) {
		status = k_sem_take(semaphore, K_NO_WAIT);
	} else {
		status = k_sem_take(semaphore, K_MSEC(timeout));
	}

	/* If k_sem_take is successful, return the number of available
	 * tokens + 1. +1 is for accounting the currently acquired token.
	 * If it has timed out, return 0 (no tokens available).
	 */
	if (status == 0) {
		return k_sem_count_get(semaphore) + 1;
	} else if (status == -EBUSY || status == -EAGAIN) {
		return 0;
	} else {
		return -1;
	}
}