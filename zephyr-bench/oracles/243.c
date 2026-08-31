osStatus osMailPut(osMailQId queue_id, void *mail)
{
	osMailQDef_t *queue_def = (osMailQDef_t *)queue_id;
	struct k_mbox_msg mmsg;

	if (queue_def == NULL) {
		return osErrorParameter;
	}

	if (mail == NULL) {
		return osErrorValue;
	}

	(void)memset(&mmsg, 0, sizeof(mmsg));
	mmsg.tx_data = mail;
	mmsg.rx_source_thread = K_ANY;
	mmsg.tx_target_thread = K_ANY;

	k_mbox_async_put(queue_def->mbox, &mmsg, NULL);
	return osOK;
}