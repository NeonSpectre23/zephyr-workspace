osMutexId_t osMutexNew(const osMutexAttr_t *attr)
{
	struct cmsis_rtos_mutex_cb *mutex;

	if (k_is_in_isr()) {
		return NULL;
	}

	if (attr == NULL) {
		attr = &init_mutex_attrs;
	}

	__ASSERT(attr->attr_bits & osMutexPrioInherit,
		 "Zephyr supports osMutexPrioInherit by default. Do not unselect it\n");

	__ASSERT(!(attr->attr_bits & osMutexRobust), "Zephyr does not support osMutexRobust.\n");

	if (attr->cb_mem != NULL) {
		__ASSERT(attr->cb_size == sizeof(struct cmsis_rtos_mutex_cb), "Invalid cb_size\n");
		mutex = (struct cmsis_rtos_mutex_cb *)attr->cb_mem;
	} else if (k_mem_slab_alloc(&cmsis_rtos_mutex_cb_slab, (void **)&mutex, K_MSEC(100)) != 0) {
		return NULL;
	}
	memset(mutex, 0, sizeof(struct cmsis_rtos_mutex_cb));
	mutex->is_cb_dynamic_allocation = attr->cb_mem == NULL;

	k_mutex_init(&mutex->z_mutex);
	mutex->state = attr->attr_bits;

	mutex->name = (attr->name == NULL) ? init_mutex_attrs.name : attr->name;

	return (osMutexId_t)mutex;
}