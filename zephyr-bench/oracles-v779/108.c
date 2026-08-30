void k_work_queue_init(struct k_work_q *queue)
{
	__ASSERT_NO_MSG(queue != NULL);

	*queue = (struct k_work_q) {
		.flags = 0,
	};

	SYS_PORT_TRACING_OBJ_INIT(k_work_queue, queue);
}