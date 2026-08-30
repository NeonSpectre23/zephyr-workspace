int k_work_submit(struct k_work *work)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_work, submit, work);

	int ret = k_work_submit_to_queue(&k_sys_work_q, work);

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_work, submit, work, ret);

	return ret;
}