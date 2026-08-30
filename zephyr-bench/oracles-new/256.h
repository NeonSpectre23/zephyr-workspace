static inline void sys_notify_init_spinwait(struct sys_notify *notify)
{
	__ASSERT_NO_MSG(notify != NULL);

	*notify = (struct sys_notify){
		.flags = SYS_NOTIFY_METHOD_SPINWAIT,
	};
}