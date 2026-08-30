uint32_t osMemoryPoolGetCapacity(osMemoryPoolId_t mp_id)
{
	struct cmsis_rtos_mempool_cb *mslab = (struct cmsis_rtos_mempool_cb *)mp_id;

	if (mslab == NULL) {
		return 0;
	} else {
		return mslab->z_mslab.info.num_blocks;
	}
}