void net_timeout_set(struct net_timeout *timeout,
		     uint32_t lifetime,
		     uint32_t now)
{
	uint64_t expire_timeout;

	timeout->timer_start = now;

	/* Highly unlikely, but a zero timeout isn't correctly handled by the
	 * standard calculation.
	 */
	if (lifetime == 0U) {
		timeout->wrap_counter = 0;
		timeout->timer_timeout = 0;
		return;
	}

	expire_timeout = (uint64_t)MSEC_PER_SEC * (uint64_t)lifetime;
	timeout->wrap_counter = expire_timeout /
		(uint64_t)NET_TIMEOUT_MAX_VALUE;
	timeout->timer_timeout = expire_timeout -
		(uint64_t)NET_TIMEOUT_MAX_VALUE *
		(uint64_t)timeout->wrap_counter;

	/* The implementation requires that the fractional timeout be zero
	 * only when the timeout has completed, so if the residual is zero
	 * copy over one timeout from the wrap.
	 */
	if (timeout->timer_timeout == 0U) {
		timeout->timer_timeout = NET_TIMEOUT_MAX_VALUE;
		timeout->wrap_counter -= 1;
	}
}