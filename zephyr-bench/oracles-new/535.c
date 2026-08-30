osStatus_t osMutexAcquire(osMutexId_t mutex_id, uint32_t timeout)
{
	struct cmsis_rtos_mutex_cb *mutex = (struct cmsis_rtos_mutex_cb *)mutex_id;
	int status;

	if (mutex_id == NULL) {
		return osErrorParameter;
	}

	if (k_is_in_isr()) {
		return osErrorISR;
	}

	if (timeout == osWaitForever) {
		status = k_mutex_lock(&mutex->z_mutex, K_FOREVER);
	} else if (timeout == 0U) {
		status = k_mutex_lock(&mutex->z_mutex, K_NO_WAIT);
	} else {
		status = k_mutex_lock(&mutex->z_mutex, K_TICKS(timeout));
	}

	if (timeout != 0 && (status == -EAGAIN || status == -EBUSY)) {
		return osErrorTimeout;
	} else if (status != 0) {
		return osErrorResource;
	} else {
		return osOK;
	}
}