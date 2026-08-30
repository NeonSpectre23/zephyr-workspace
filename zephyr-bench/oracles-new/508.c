void *osMailAlloc(osMailQId queue_id, uint32_t millisec)
{
	osMailQDef_t *queue_def = (osMailQDef_t *)queue_id;
	char *ptr;
	int retval;

	if (queue_def == NULL) {
		return NULL;
	}

	if (millisec == 0U) {
		retval = k_mem_slab_alloc(
				(struct k_mem_slab *)(queue_def->pool),
				(void **)&ptr, K_NO_WAIT);
	} else if (millisec == osWaitForever) {
		retval = k_mem_slab_alloc(
				(struct k_mem_slab *)(queue_def->pool),
				(void **)&ptr, K_FOREVER);
	} else {
		retval = k_mem_slab_alloc(
				(struct k_mem_slab *)(queue_def->pool),
				(void **)&ptr, K_MSEC(millisec));
	}

	if (retval == 0) {
		return ptr;
	} else {
		return NULL;
	}
}