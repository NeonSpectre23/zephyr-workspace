osSemaphoreId osSemaphoreCreate(const osSemaphoreDef_t *semaphore_def,
				int32_t count)
{
	struct k_sem *semaphore;

	if (semaphore_def == NULL) {
		return NULL;
	}

	if (k_is_in_isr()) {
		return NULL;
	}

	if (k_mem_slab_alloc(&cmsis_semaphore_slab,
				(void **)&semaphore, K_MSEC(100)) == 0) {
		(void)memset(semaphore, 0, sizeof(struct k_sem));
	} else {
		return NULL;
	}

	k_sem_init(semaphore, count, count);

	return (osSemaphoreId)semaphore;
}