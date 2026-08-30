osStatus_t osMessageQueueReset(osMessageQueueId_t msgq_id)
{
	struct cmsis_rtos_msgq_cb *msgq = (struct cmsis_rtos_msgq_cb *)msgq_id;

	if (msgq == NULL) {
		return osErrorParameter;
	}

	if (k_is_in_isr()) {
		return osErrorISR;
	}

	/* The status code "osErrorResource" (the message queue specified by
	 * parameter msgq_id is in an invalid message queue state) is not
	 * supported in Zephyr.
	 */

	k_msgq_purge(&msgq->z_msgq);

	return osOK;
}