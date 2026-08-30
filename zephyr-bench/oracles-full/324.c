osStatus osMutexRelease(osMutexId mutex_id)
{
	struct k_mutex *mutex = (struct k_mutex *) mutex_id;

	if (mutex_id == NULL) {
		return osErrorParameter;
	}

	if (k_is_in_isr()) {
		return osErrorISR;
	}

	if (k_mutex_unlock(mutex) != 0) {
		return osErrorResource;
	}

	return osOK;
}