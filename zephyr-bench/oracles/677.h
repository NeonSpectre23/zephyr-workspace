static inline bool sys_notify_uses_callback(const struct sys_notify *notify)
{
	__ASSERT_NO_MSG(notify != NULL);

	return sys_notify_get_method(notify) == SYS_NOTIFY_METHOD_CALLBACK;
}