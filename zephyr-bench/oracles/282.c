osThreadId_t osThreadNew(osThreadFunc_t threadfunc, void *arg, const osThreadAttr_t *attr)
{
	int32_t prio;
	osPriority_t cv2_prio;
	struct cmsis_rtos_thread_cb *tid;
	static uint32_t one_time;
	void *stack;
	size_t stack_size;

	if (k_is_in_isr()) {
		return NULL;
	}

	if (attr == NULL) {
		attr = &init_thread_attrs;
	}

	if (attr->priority == osPriorityNone) {
		cv2_prio = osPriorityNormal;
	} else {
		cv2_prio = attr->priority;
	}

	if (attr->cb_mem == NULL && num_dynamic_cb >= CONFIG_CMSIS_V2_THREAD_MAX_COUNT) {
		return NULL;
	}

	if (attr->stack_mem == NULL &&
	    num_dynamic_stack >= CONFIG_CMSIS_V2_THREAD_DYNAMIC_MAX_COUNT) {
		return NULL;
	}

	BUILD_ASSERT(osPriorityISR <= CONFIG_NUM_PREEMPT_PRIORITIES,
		     "Configure NUM_PREEMPT_PRIORITIES to at least osPriorityISR");

	BUILD_ASSERT(CONFIG_CMSIS_V2_THREAD_DYNAMIC_STACK_SIZE <=
			     CONFIG_CMSIS_V2_THREAD_MAX_STACK_SIZE,
		     "Default dynamic thread stack size cannot exceed max stack size");

	__ASSERT(attr->stack_size <= CONFIG_CMSIS_V2_THREAD_MAX_STACK_SIZE, "invalid stack size\n");

	__ASSERT((cv2_prio >= osPriorityIdle) && (cv2_prio <= osPriorityISR), "invalid priority\n");

	if (attr->stack_mem != NULL && attr->stack_size == 0) {
		return NULL;
	}

#if CONFIG_CMSIS_V2_THREAD_MAX_COUNT != 0
	if (attr->cb_mem == NULL) {
		uint32_t this_dynamic_cb;
		this_dynamic_cb = atomic_inc(&num_dynamic_cb);
		tid = &cmsis_rtos_thread_cb_pool[this_dynamic_cb];
	} else
#endif
	{
		tid = (struct cmsis_rtos_thread_cb *)attr->cb_mem;
	}

	tid->attr_bits = attr->attr_bits;

#if CONFIG_CMSIS_V2_THREAD_DYNAMIC_MAX_COUNT != 0
	if (attr->stack_mem == NULL) {
		uint32_t this_dynamic_stack;
		__ASSERT(CONFIG_CMSIS_V2_THREAD_DYNAMIC_STACK_SIZE > 0,
			 "dynamic stack size must be configured to be non-zero\n");
		this_dynamic_stack = atomic_inc(&num_dynamic_stack);
		stack_size = CONFIG_CMSIS_V2_THREAD_DYNAMIC_STACK_SIZE;
		stack = cmsis_rtos_thread_stack_pool[this_dynamic_stack];
	} else
#endif
	{
		stack_size = attr->stack_size;
		stack = attr->stack_mem;
	}

	k_poll_signal_init(&tid->poll_signal);
	k_poll_event_init(&tid->poll_event, K_POLL_TYPE_SIGNAL, K_POLL_MODE_NOTIFY_ONLY,
			  &tid->poll_signal);
	tid->signal_results = 0U;

	/* TODO: Do this somewhere only once */
	if (one_time == 0U) {
		sys_dlist_init(&thread_list);
		one_time = 1U;
	}

	sys_dlist_append(&thread_list, &tid->node);

	prio = cmsis_to_zephyr_priority(cv2_prio);

	(void)k_thread_create(&tid->z_thread, stack, stack_size, zephyr_thread_wrapper, (void *)arg,
			      NULL, threadfunc, prio, 0, K_NO_WAIT);

	const char *name = (attr->name == NULL) ? init_thread_attrs.name : attr->name;

	k_thread_name_set(&tid->z_thread, name);

	return (osThreadId_t)tid;
}