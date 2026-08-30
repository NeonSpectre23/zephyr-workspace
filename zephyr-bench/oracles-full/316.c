osMessageQueueId_t osMessageQueueNew(uint32_t msg_count, uint32_t msg_size,
				     const osMessageQueueAttr_t *attr)
{
	struct cmsis_rtos_msgq_cb *msgq;

	BUILD_ASSERT(K_HEAP_MEM_POOL_SIZE >= CONFIG_CMSIS_V2_MSGQ_MAX_DYNAMIC_SIZE,
		     "heap must be configured to be at least the max dynamic size");

	if (k_is_in_isr()) {
		return NULL;
	}

	if ((attr != NULL) && (attr->mq_size < msg_count * msg_size)) {
		return NULL;
	}

	if (attr == NULL) {
		attr = &init_msgq_attrs;
	}

	if (attr->cb_mem != NULL) {
		__ASSERT(attr->cb_size == sizeof(struct cmsis_rtos_msgq_cb), "Invalid cb_size\n");
		msgq = (struct cmsis_rtos_msgq_cb *)attr->cb_mem;
	} else if (k_mem_slab_alloc(&cmsis_rtos_msgq_cb_slab, (void **)&msgq, K_MSEC(100)) != 0) {
		return NULL;
	}
	(void)memset(msgq, 0, sizeof(struct cmsis_rtos_msgq_cb));
	msgq->is_cb_dynamic_allocation = attr->cb_mem == NULL;

	if (attr->mq_mem == NULL) {
		__ASSERT((msg_count * msg_size) <= CONFIG_CMSIS_V2_MSGQ_MAX_DYNAMIC_SIZE,
			 "message queue size exceeds dynamic maximum");

#if (K_HEAP_MEM_POOL_SIZE > 0)
		msgq->pool = k_calloc(msg_count, msg_size);
		if (msgq->pool == NULL) {
			if (msgq->is_cb_dynamic_allocation) {
				k_mem_slab_free(&cmsis_rtos_msgq_cb_slab, (void *)msgq);
			}
			return NULL;
		}
		msgq->is_dynamic_allocation = TRUE;
#else
		if (msgq->is_cb_dynamic_allocation) {
			k_mem_slab_free(&cmsis_rtos_msgq_cb_slab, (void *)msgq);
		}
		return NULL;
#endif
	} else {
		msgq->pool = attr->mq_mem;
		msgq->is_dynamic_allocation = FALSE;
	}

	k_msgq_init(&msgq->z_msgq, msgq->pool, msg_size, msg_count);

	msgq->name = (attr->name == NULL) ? init_msgq_attrs.name : attr->name;

	return (osMessageQueueId_t)(msgq);
}