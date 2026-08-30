osStatus_t osMemoryPoolFree(osMemoryPoolId_t mp_id, void *block)
{
	struct cmsis_rtos_mempool_cb *mslab = (struct cmsis_rtos_mempool_cb *)mp_id;

	if (mslab == NULL) {
		return osErrorParameter;
	}

	/* Note: Below error code is not supported.
	 *       osErrorResource: the memory pool specified by parameter mp_id
	 *       is in an invalid memory pool state.
	 */
	if (mslab->is_cb_dynamic_allocation) {
		k_mem_slab_free((struct k_mem_slab *)(&mslab->z_mslab), (void *)block);
	}
	return osOK;
}