osThreadId osThreadCreate(const osThreadDef_t *thread_def, void *arg)
{
	struct k_thread *cm_thread;
	uint32_t prio;
	k_tid_t tid;
	uint32_t stacksz;
	int ret;
	size_t instance;

	k_thread_stack_t
	   (*stk_ptr)[K_THREAD_STACK_LEN(CONFIG_CMSIS_THREAD_MAX_STACK_SIZE)];

	if ((thread_def == NULL) || (thread_def->instances == 0)) {
		return NULL;
	}

	BUILD_ASSERT(
		CONFIG_NUM_PREEMPT_PRIORITIES >= TOTAL_CMSIS_THREAD_PRIORITIES,
		"Configure NUM_PREEMPT_PRIORITIES to at least"
		" TOTAL_CMSIS_THREAD_PRIORITIES");

	__ASSERT(thread_def->stacksize <= CONFIG_CMSIS_THREAD_MAX_STACK_SIZE,
		 "invalid stack size\n");

	if (k_is_in_isr()) {
		return NULL;
	}

	__ASSERT((thread_def->tpriority >= osPriorityIdle) &&
		 (thread_def->tpriority <= osPriorityRealtime),
		 "invalid priority\n");

	/* get an available thread instance */
	ret = sys_bitarray_alloc((sys_bitarray_t *)(thread_def->status_mask),
			1, &instance);
	if (ret != 0) {
		return NULL;
	}

	stacksz = thread_def->stacksize;
	if (stacksz == 0U) {
		stacksz = CONFIG_CMSIS_THREAD_MAX_STACK_SIZE;
	}

	k_poll_signal_init(thread_def->poll_signal);
	k_poll_event_init(thread_def->poll_event, K_POLL_TYPE_SIGNAL,
			K_POLL_MODE_NOTIFY_ONLY, thread_def->poll_signal);

	cm_thread = thread_def->cm_thread;
	stk_ptr = thread_def->stack_mem;
	prio = cmsis_to_zephyr_priority(thread_def->tpriority);
	k_thread_custom_data_set((void *)thread_def);

	tid = k_thread_create(&cm_thread[instance],
			stk_ptr[instance], stacksz,
			zephyr_thread_wrapper,
			(void *)arg, NULL, thread_def->pthread,
			prio, 0, K_NO_WAIT);

	/* make custom_data pointer of thread point to its source thread_def,
	 * then we can use it to release thread instances
	 * when terminating threads
	 */
	tid->custom_data = (void *)thread_def;

	return ((osThreadId)tid);
}