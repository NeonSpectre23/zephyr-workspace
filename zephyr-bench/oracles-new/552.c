osSemaphoreId_t osSemaphoreNew(uint32_t max_count, uint32_t initial_count,
			       const osSemaphoreAttr_t *attr)
{
	struct cmsis_rtos_semaphore_cb *semaphore;

	if (k_is_in_isr()) {
		return NULL;
	}

	if (attr == NULL) {
		attr = &init_sema_attrs;
	}

	if (attr->cb_mem != NULL) {
		__ASSERT(attr->cb_size == sizeof(struct cmsis_rtos_semaphore_cb),
			 "Invalid cb_size\n");
		semaphore = (struct cmsis_rtos_semaphore_cb *)attr->cb_mem;
	} else if (k_mem_slab_alloc(&cmsis_rtos_semaphore_cb_slab, (void **)&semaphore,
				    K_MSEC(100)) != 0) {
		return NULL;
	}
	(void)memset(semaphore, 0, sizeof(struct cmsis_rtos_semaphore_cb));
	semaphore->is_cb_dynamic_allocation = attr->cb_mem == NULL;

	k_sem_init(&semaphore->z_semaphore, initial_count, max_count);

	semaphore->name = (attr->name == NULL) ? init_sema_attrs.name : attr->name;

	return (osSemaphoreId_t)semaphore;
}