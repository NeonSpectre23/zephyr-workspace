osEventFlagsId_t osEventFlagsNew(const osEventFlagsAttr_t *attr)
{
	struct cmsis_rtos_event_cb *events;

	if (k_is_in_isr()) {
		return NULL;
	}

	if (attr == NULL) {
		attr = &init_event_flags_attrs;
	}

	if (attr->cb_mem != NULL) {
		__ASSERT(attr->cb_size == sizeof(struct cmsis_rtos_event_cb), "Invalid cb_size\n");
		events = (struct cmsis_rtos_event_cb *)attr->cb_mem;
	} else if (k_mem_slab_alloc(&cmsis_rtos_event_cb_slab, (void **)&events, K_MSEC(100)) !=
		   0) {
		return NULL;
	}
	memset(events, 0, sizeof(struct cmsis_rtos_event_cb));

	k_event_init(&events->z_event);
	events->is_cb_dynamic_allocation = (attr->cb_mem == NULL);
	events->name = (attr->name == NULL) ? init_event_flags_attrs.name : attr->name;

	return (osEventFlagsId_t)events;
}