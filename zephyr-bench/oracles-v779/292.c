const char *osSemaphoreGetName(osSemaphoreId_t semaphore_id)
{
	struct cmsis_rtos_semaphore_cb *semaphore = (struct cmsis_rtos_semaphore_cb *)semaphore_id;

	if (semaphore == NULL) {
		return NULL;
	}
	return semaphore->name;
}