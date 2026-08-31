osMemoryPoolId_t osMemoryPoolNew(uint32_t block_count, uint32_t block_size,
				 const osMemoryPoolAttr_t *attr)
{
	struct cmsis_rtos_mempool_cb *mslab;

	BUILD_ASSERT(K_HEAP_MEM_POOL_SIZE >= CONFIG_CMSIS_V2_MEM_SLAB_MAX_DYNAMIC_SIZE,
		     "heap must be configured to be at least the max dynamic size");

	if (k_is_in_isr()) {
		return NULL;
	}

	if ((attr != NULL) && (attr->mp_size < block_count * block_size)) {
		return NULL;
	}

	if (attr == NULL) {
		attr = &init_mslab_attrs;
	}

	if (attr->cb_mem != NULL) {
		__ASSERT(attr->cb_size == sizeof(struct cmsis_rtos_mempool_cb),
			 "Invalid cb_size\n");
		mslab = (struct cmsis_rtos_mempool_cb *)attr->cb_mem;
	} else if (k_mem_slab_alloc(&cv2_mem_slab, (void **)&mslab, K_MSEC(100)) != 0) {
		return NULL;
	}
	(void)memset(mslab, 0, sizeof(struct cmsis_rtos_mempool_cb));
	mslab->is_cb_dynamic_allocation = attr->cb_mem == NULL;

	if (attr->mp_mem == NULL) {
		__ASSERT((block_count * block_size) <= CONFIG_CMSIS_V2_MEM_SLAB_MAX_DYNAMIC_SIZE,
			 "memory slab/pool size exceeds dynamic maximum");

		mslab->pool = k_calloc(block_count, block_size);
		if (mslab->pool == NULL) {
			if (mslab->is_cb_dynamic_allocation) {
				k_mem_slab_free(&cv2_mem_slab, (void *)mslab);
			}
			return NULL;
		}
		mslab->is_dynamic_allocation = TRUE;
	} else {
		mslab->pool = attr->mp_mem;
		mslab->is_dynamic_allocation = FALSE;
	}

	int rc = k_mem_slab_init(&mslab->z_mslab, mslab->pool, block_size, block_count);
	if (rc != 0) {
		if (mslab->is_cb_dynamic_allocation) {
			k_mem_slab_free(&cv2_mem_slab, (void *)mslab);
		}
		if (attr->mp_mem == NULL) {
			k_free(mslab->pool);
		}
		return NULL;
	}

	mslab->name = (attr->name == NULL) ? init_mslab_attrs.name : attr->name;

	return (osMemoryPoolId_t)mslab;
}