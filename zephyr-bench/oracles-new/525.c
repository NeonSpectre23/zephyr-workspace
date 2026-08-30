osStatus osMessagePut(osMessageQId queue_id, uint32_t info, uint32_t millisec)
{
	osMessageQDef_t *queue_def = (osMessageQDef_t *)queue_id;
	int retval;

	if (queue_def == NULL) {
		return osErrorParameter;
	}

	if (millisec == 0U) {
		retval = k_msgq_put(queue_def->msgq, (void *)&info, K_NO_WAIT);
	} else if (millisec == osWaitForever) {
		retval = k_msgq_put(queue_def->msgq, (void *)&info, K_FOREVER);
	} else {
		retval = k_msgq_put(queue_def->msgq, (void *)&info,
				    K_MSEC(millisec));
	}

	if (retval == 0) {
		return osOK;
	} else if (retval == -EAGAIN) {
		return osErrorTimeoutResource;
	} else {
		return osErrorResource;
	}
}