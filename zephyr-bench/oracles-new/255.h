static inline void sys_notify_init_signal(struct sys_notify *notify,
					    struct k_poll_signal *sigp)
{
	__ASSERT_NO_MSG(notify != NULL);
	__ASSERT_NO_MSG(sigp != NULL);

	*notify = (struct sys_notify){
		.method = {
			.signal = sigp,
		},
		.flags = SYS_NOTIFY_METHOD_SIGNAL,
	};
}