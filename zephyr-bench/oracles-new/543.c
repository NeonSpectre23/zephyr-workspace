void *osPoolAlloc(osPoolId pool_id)
{
	osPoolDef_t *osPool = (osPoolDef_t *)pool_id;
	void *ptr;

	if (k_mem_slab_alloc((struct k_mem_slab *)(osPool->pool),
				&ptr, TIME_OUT) == 0) {
		return ptr;
	} else {
		return NULL;
	}
}