uint32_t osMessageQueueGetCapacity(osMessageQueueId_t msgq_id)
{
	struct cmsis_rtos_msgq_cb *msgq = (struct cmsis_rtos_msgq_cb *)msgq_id;

	if (msgq == NULL) {
		return 0;
	} else {
		return msgq->z_msgq.max_msgs;
	}
}