const char *osMemoryPoolGetName(osMemoryPoolId_t mp_id)
{
	struct cmsis_rtos_mempool_cb *mslab = (struct cmsis_rtos_mempool_cb *)mp_id;

	if (mslab == NULL) {
		return NULL;
	}
	return mslab->name;
}