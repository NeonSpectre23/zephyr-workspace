osStatus osSemaphoreDelete(osSemaphoreId semaphore_id)
{
	struct k_sem *semaphore = (struct k_sem *) semaphore_id;

	if (semaphore_id == NULL) {
		return osErrorParameter;
	}

	if (k_is_in_isr()) {
		return osErrorISR;
	}

	/* The status code "osErrorResource" (the semaphore object
	 * could not be deleted) is not supported in Zephyr.
	 */

	k_mem_slab_free(&cmsis_semaphore_slab, (void *)semaphore);

	return osOK;
}