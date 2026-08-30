osStatus osPoolFree(osPoolId pool_id, void *block)
{
	osPoolDef_t *osPool = (osPoolDef_t *)pool_id;

	/* Note: Below 2 error codes are not supported.
	 *       osErrorValue: block does not belong to the memory pool.
	 *       osErrorParameter: a parameter is invalid or outside of a
	 *                         permitted range.
	 */

	k_mem_slab_free((struct k_mem_slab *)(osPool->pool), (void *)block);

	return osOK;
}