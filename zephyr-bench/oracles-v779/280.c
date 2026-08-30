uint32_t osMessageQueueGetCount(osMessageQueueId_t msgq_id)
{
	struct cmsis_rtos_msgq_cb *msgq = (struct cmsis_rtos_msgq_cb *)msgq_id;

	if (msgq == NULL) {
		return 0;
	} else {
		return k_msgq_num_used_get(&msgq->z_msgq);
	}
}