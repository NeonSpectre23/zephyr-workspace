osStatus osSemaphoreRelease(osSemaphoreId semaphore_id)
{
	struct k_sem *semaphore = (struct k_sem *) semaphore_id;

	if (semaphore_id == NULL) {
		return osErrorParameter;
	}

	/* All tokens have already been released */
	if (k_sem_count_get(semaphore) == semaphore->limit) {
		return osErrorResource;
	}

	k_sem_give(semaphore);

	return osOK;
}