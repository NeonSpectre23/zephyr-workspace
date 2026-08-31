static inline void sys_notify_init_callback(struct sys_notify *notify,
					      sys_notify_generic_callback handler)
{
	__ASSERT_NO_MSG(notify != NULL);
	__ASSERT_NO_MSG(handler != NULL);

	*notify = (struct sys_notify){
		.method = {
			.callback = handler,
		},
		.flags = SYS_NOTIFY_METHOD_CALLBACK,
	};
}