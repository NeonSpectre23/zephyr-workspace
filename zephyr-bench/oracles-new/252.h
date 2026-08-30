static inline int sys_notify_fetch_result(const struct sys_notify *notify,
					    int *result)
{
	__ASSERT_NO_MSG(notify != NULL);
	__ASSERT_NO_MSG(result != NULL);
	int rv = -EAGAIN;

	if (sys_notify_get_method(notify) == SYS_NOTIFY_METHOD_COMPLETED) {
		rv = 0;
		*result = notify->result;
	}

	return rv;
}