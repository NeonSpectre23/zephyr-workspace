osEvent osMessageGet(osMessageQId queue_id, uint32_t millisec)
{
	osMessageQDef_t *queue_def = (osMessageQDef_t *)queue_id;
	uint32_t info;
	osEvent evt = {0};
	int retval;

	if (queue_def == NULL) {
		evt.status = osErrorParameter;
		return evt;
	}

	if (millisec == 0U) {
		retval = k_msgq_get(queue_def->msgq, &info, K_NO_WAIT);
	} else if (millisec == osWaitForever) {
		retval = k_msgq_get(queue_def->msgq, &info, K_FOREVER);
	} else {
		retval = k_msgq_get(queue_def->msgq, &info, K_MSEC(millisec));
	}

	if (retval == 0) {
		evt.status = osEventMessage;
		evt.value.v = info;
	} else if (retval == -EAGAIN) {
		evt.status = osEventTimeout;
	} else if (retval == -ENOMSG) {
		evt.status = osOK;
	}

	evt.def.message_id = queue_id;

	return evt;
}