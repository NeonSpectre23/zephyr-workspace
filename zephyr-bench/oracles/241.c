osMailQId osMailCreate(const osMailQDef_t *queue_def, osThreadId thread_id)
{
	if (queue_def == NULL) {
		return NULL;
	}

	if (k_is_in_isr()) {
		return NULL;
	}

	k_mbox_init(queue_def->mbox);
	return (osMailQId)(queue_def);
}