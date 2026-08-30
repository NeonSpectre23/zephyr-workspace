osEvent osMailGet(osMailQId queue_id, uint32_t millisec)
{
	osMailQDef_t *queue_def = (osMailQDef_t *)queue_id;
	struct k_mbox_msg mmsg;
	osEvent evt = {0};
	int retval;

	if (queue_def == NULL) {
		evt.status = osErrorParameter;
		return evt;
	}

	(void)memset(&mmsg, 0, sizeof(mmsg));
	mmsg.rx_source_thread = K_ANY;
	mmsg.tx_target_thread = K_ANY;

	if (millisec == 0U) {
		retval = k_mbox_get(queue_def->mbox, &mmsg, NULL, K_NO_WAIT);
	} else if (millisec == osWaitForever) {
		retval = k_mbox_get(queue_def->mbox, &mmsg, NULL, K_FOREVER);
	} else {
		retval = k_mbox_get(queue_def->mbox, &mmsg, NULL,
				    K_MSEC(millisec));
	}

	if (retval == 0) {
		evt.status = osEventMail;
		evt.value.p = mmsg.tx_data;
	} else if (retval == -EAGAIN) {
		evt.status = osEventTimeout;
	} else if (retval == -ENOMSG) {
		evt.status = osOK;
	} else {
		evt.status = osErrorValue;
	}

	evt.def.mail_id = queue_id;

	return evt;
}