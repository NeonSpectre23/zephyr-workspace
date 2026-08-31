osTimerId_t osTimerNew(osTimerFunc_t func, osTimerType_t type, void *argument,
		       const osTimerAttr_t *attr)
{
	struct cmsis_rtos_timer_cb *timer;

	if (type != osTimerOnce && type != osTimerPeriodic) {
		return NULL;
	}

	if (k_is_in_isr()) {
		return NULL;
	}

	if (attr == NULL) {
		attr = &init_timer_attrs;
	}

	if (attr->cb_mem != NULL) {
		__ASSERT(attr->cb_size == sizeof(struct cmsis_rtos_timer_cb), "Invalid cb_size\n");
		timer = (struct cmsis_rtos_timer_cb *)attr->cb_mem;
	} else if (k_mem_slab_alloc(&cmsis_rtos_timer_cb_slab, (void **)&timer, K_MSEC(100)) != 0) {
		return NULL;
	}
	(void)memset(timer, 0, sizeof(struct cmsis_rtos_timer_cb));
	timer->is_cb_dynamic_allocation = attr->cb_mem == NULL;

	timer->callback_function = func;
	timer->arg = argument;
	timer->type = type;
	timer->status = NOT_ACTIVE;

	k_timer_init(&timer->z_timer, zephyr_timer_wrapper, NULL);

	timer->name = (attr->name == NULL) ? init_timer_attrs.name : attr->name;

	return (osTimerId_t)timer;
}