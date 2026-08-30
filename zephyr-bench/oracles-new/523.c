osMessageQId osMessageCreate(const osMessageQDef_t *queue_def,
				osThreadId thread_id)
{
	if (queue_def == NULL) {
		return NULL;
	}

	if (k_is_in_isr()) {
		return NULL;
	}

	k_msgq_init(queue_def->msgq, queue_def->pool,
			queue_def->item_sz, queue_def->queue_sz);
	return (osMessageQId)(queue_def);
}