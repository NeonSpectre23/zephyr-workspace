osStatus_t osMemoryPoolDelete(osMemoryPoolId_t mp_id)
{
	struct cmsis_rtos_mempool_cb *mslab = (struct cmsis_rtos_mempool_cb *)mp_id;

	if (mslab == NULL) {
		return osErrorParameter;
	}

	if (k_is_in_isr()) {
		return osErrorISR;
	}

	/* The status code "osErrorResource" (the memory pool specified by
	 * parameter mp_id is in an invalid memory pool state) is not
	 * supported in Zephyr.
	 */

	if (mslab->is_dynamic_allocation) {
		k_free(mslab->pool);
	}
	if (mslab->is_cb_dynamic_allocation) {
		k_mem_slab_free(&cv2_mem_slab, (void *)mslab);
	}
	return osOK;
}