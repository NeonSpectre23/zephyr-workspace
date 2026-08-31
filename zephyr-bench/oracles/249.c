uint32_t osMemoryPoolGetCount(osMemoryPoolId_t mp_id)
{
	struct cmsis_rtos_mempool_cb *mslab = (struct cmsis_rtos_mempool_cb *)mp_id;

	if (mslab == NULL) {
		return 0;
	} else {
		return k_mem_slab_num_used_get(&mslab->z_mslab);
	}
}