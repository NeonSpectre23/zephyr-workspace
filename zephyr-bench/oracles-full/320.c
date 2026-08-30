osStatus osMutexDelete(osMutexId mutex_id)
{
	struct k_mutex *mutex = (struct k_mutex *) mutex_id;

	if (mutex_id == NULL) {
		return osErrorParameter;
	}

	if (k_is_in_isr()) {
		return osErrorISR;
	}

	/* The status code "osErrorResource" (mutex object could
	 * not be deleted) is not supported in Zephyr.
	 */

	k_mem_slab_free(&cmsis_mutex_slab, (void *)mutex);

	return osOK;
}