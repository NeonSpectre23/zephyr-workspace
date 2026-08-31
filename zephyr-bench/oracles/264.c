osStatus_t osSemaphoreAcquire(osSemaphoreId_t semaphore_id, uint32_t timeout)
{
	struct cmsis_rtos_semaphore_cb *semaphore = (struct cmsis_rtos_semaphore_cb *)semaphore_id;
	int status;

	if (semaphore_id == NULL) {
		return osErrorParameter;
	}

	/* Can be called from ISRs only if timeout is set to 0 */
	if (timeout > 0 && k_is_in_isr()) {
		return osErrorParameter;
	}

	if (timeout == osWaitForever) {
		status = k_sem_take(&semaphore->z_semaphore, K_FOREVER);
	} else if (timeout == 0U) {
		status = k_sem_take(&semaphore->z_semaphore, K_NO_WAIT);
	} else {
		status = k_sem_take(&semaphore->z_semaphore, K_TICKS(timeout));
	}

	if (status == -EBUSY) {
		return osErrorResource;
	} else if (status == -EAGAIN) {
		return osErrorTimeout;
	} else {
		return osOK;
	}
}