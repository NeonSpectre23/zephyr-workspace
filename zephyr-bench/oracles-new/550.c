uint32_t osSemaphoreGetCount(osSemaphoreId_t semaphore_id)
{
	struct cmsis_rtos_semaphore_cb *semaphore = (struct cmsis_rtos_semaphore_cb *)semaphore_id;

	if (semaphore_id == NULL) {
		return 0;
	}

	return k_sem_count_get(&semaphore->z_semaphore);
}