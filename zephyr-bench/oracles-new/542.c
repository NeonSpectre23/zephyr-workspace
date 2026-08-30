osStatus osMutexWait(osMutexId mutex_id, uint32_t timeout)
{
	struct k_mutex *mutex = (struct k_mutex *) mutex_id;
	int status;

	if (mutex_id == NULL) {
		return osErrorParameter;
	}

	if (k_is_in_isr()) {
		return osErrorISR;
	}

	if (timeout == osWaitForever) {
		status = k_mutex_lock(mutex, K_FOREVER);
	} else if (timeout == 0U) {
		status = k_mutex_lock(mutex, K_NO_WAIT);
	} else {
		status = k_mutex_lock(mutex, K_MSEC(timeout));
	}

	if (timeout != 0 && (status == -EAGAIN || status == -EBUSY)) {
		return osErrorTimeoutResource;
	} else if (status == 0) {
		return osOK;
	} else {
		return osErrorResource;
	}
}