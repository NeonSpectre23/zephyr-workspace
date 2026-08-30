osStatus_t osMessageQueuePut(osMessageQueueId_t msgq_id, const void *msg_ptr, uint8_t msg_prio,
			     uint32_t timeout)
{
	struct cmsis_rtos_msgq_cb *msgq = (struct cmsis_rtos_msgq_cb *)msgq_id;
	int retval;

	ARG_UNUSED(msg_prio);

	if (msgq == NULL) {
		return osErrorParameter;
	}

	/* Can be called from ISRs only if timeout is set to 0 */
	if (timeout > 0 && k_is_in_isr()) {
		return osErrorParameter;
	}

	if (timeout == 0U) {
		retval = k_msgq_put(&msgq->z_msgq, (void *)msg_ptr, K_NO_WAIT);
	} else if (timeout == osWaitForever) {
		retval = k_msgq_put(&msgq->z_msgq, (void *)msg_ptr, K_FOREVER);
	} else {
		retval = k_msgq_put(&msgq->z_msgq, (void *)msg_ptr, K_TICKS(timeout));
	}

	if (retval == 0) {
		return osOK;
	} else if (retval == -EAGAIN) {
		return osErrorTimeout;
	} else {
		return osErrorResource;
	}
}