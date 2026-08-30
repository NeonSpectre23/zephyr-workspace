static inline uint32_t sys_notify_get_method(const struct sys_notify *notify)
{
	uint32_t method = notify->flags >> SYS_NOTIFY_METHOD_POS;

	return method & SYS_NOTIFY_METHOD_MASK;
}