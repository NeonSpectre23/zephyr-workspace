uint32_t net_timeout_remaining(const struct net_timeout *timeout,
			       uint32_t now)
{
	int64_t ret = timeout->timer_timeout;

	ret += timeout->wrap_counter * (uint64_t)NET_TIMEOUT_MAX_VALUE;
	ret -= (int64_t)(int32_t)(now - timeout->timer_start);
	if (ret <= 0) {
		return 0;
	}

	return (uint32_t)((uint64_t)ret / MSEC_PER_SEC);
}