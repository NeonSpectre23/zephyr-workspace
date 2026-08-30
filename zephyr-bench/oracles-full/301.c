void *osMemoryPoolAlloc(osMemoryPoolId_t mp_id, uint32_t timeout)
{
	struct cmsis_rtos_mempool_cb *mslab = (struct cmsis_rtos_mempool_cb *)mp_id;
	int retval;
	void *ptr;

	if (mslab == NULL) {
		return NULL;
	}

	/* Can be called from ISRs only if timeout is set to 0 */
	if (timeout > 0 && k_is_in_isr()) {
		return NULL;
	}

	if (timeout == 0U) {
		retval = k_mem_slab_alloc((struct k_mem_slab *)(&mslab->z_mslab), (void **)&ptr,
					  K_NO_WAIT);
	} else if (timeout == osWaitForever) {
		retval = k_mem_slab_alloc((struct k_mem_slab *)(&mslab->z_mslab), (void **)&ptr,
					  K_FOREVER);
	} else {
		retval = k_mem_slab_alloc((struct k_mem_slab *)(&mslab->z_mslab), (void **)&ptr,
					  K_TICKS(timeout));
	}

	if (retval == 0) {
		return ptr;
	} else {
		return NULL;
	}
}