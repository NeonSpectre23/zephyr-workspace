void k_work_init(struct k_work *work,
		  k_work_handler_t handler)
{
	__ASSERT_NO_MSG(work != NULL);
	__ASSERT_NO_MSG(handler != NULL);

	*work = (struct k_work)Z_WORK_INITIALIZER(handler);

	SYS_PORT_TRACING_OBJ_INIT(k_work, work);
}