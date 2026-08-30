osMutexId osMutexCreate(const osMutexDef_t *mutex_def)
{
	struct k_mutex *mutex;

	if (mutex_def == NULL) {
		return NULL;
	}

	if (k_is_in_isr()) {
		return NULL;
	}

	if (k_mem_slab_alloc(&cmsis_mutex_slab, (void **)&mutex, K_MSEC(100)) == 0) {
		(void)memset(mutex, 0, sizeof(struct k_mutex));
	} else {
		return NULL;
	}

	k_mutex_init(mutex);

	return (osMutexId)mutex;
}