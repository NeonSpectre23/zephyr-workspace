int sys_notify_validate(struct sys_notify *notify)
{
	int rv = 0;

	if (notify == NULL) {
		return -EINVAL;
	}

	/* Validate configuration based on mode */
	switch (sys_notify_get_method(notify)) {
	case SYS_NOTIFY_METHOD_SPINWAIT:
		break;
	case SYS_NOTIFY_METHOD_CALLBACK:
		if (notify->method.callback == NULL) {
			rv = -EINVAL;
		}
		break;
#ifdef CONFIG_POLL
	case SYS_NOTIFY_METHOD_SIGNAL:
		if (notify->method.signal == NULL) {
			rv = -EINVAL;
		}
		break;
#endif /* CONFIG_POLL */
	default:
		rv = -EINVAL;
		break;
	}

	/* Clear the result here instead of in all callers. */
	if (rv == 0) {
		notify->result = 0;
	}

	return rv;
}