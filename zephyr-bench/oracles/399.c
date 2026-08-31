sys_notify_generic_callback sys_notify_finalize(struct sys_notify *notify,
						    int res)
{
	struct k_poll_signal *sig = NULL;
	sys_notify_generic_callback rv = NULL;
	uint32_t method = sys_notify_get_method(notify);

	/* Store the result and capture secondary notification
	 * information.
	 */
	notify->result = res;
	switch (method) {
	case SYS_NOTIFY_METHOD_SPINWAIT:
		break;
	case SYS_NOTIFY_METHOD_CALLBACK:
		rv = notify->method.callback;
		break;
	case SYS_NOTIFY_METHOD_SIGNAL:
		sig = notify->method.signal;
		break;
	default:
		__ASSERT_NO_MSG(false);
	}

	/* Mark completion by clearing the flags field to the
	 * completed state, releasing any spin-waiters, then complete
	 * secondary notification.
	 */
	compiler_barrier();
	notify->flags = SYS_NOTIFY_METHOD_COMPLETED;

	if (IS_ENABLED(CONFIG_POLL) && (sig != NULL)) {
		k_poll_signal_raise(sig, res);
	}

	return rv;
}