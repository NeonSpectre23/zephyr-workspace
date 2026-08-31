int64_t net_timeout_deadline(const struct net_timeout *timeout,
			     int64_t now)
{
	uint64_t start;
	uint64_t deadline;

	/* Reconstruct the full-precision start time assuming that the full
	 * precision start time is less than 2^32 ticks in the past.
	 */
	start = (uint64_t)now;
	start -= (uint32_t)now - timeout->timer_start;

	/* Offset the start time by the full precision remaining delay. */
	deadline = start + timeout->timer_timeout;
	deadline += (uint64_t)NET_TIMEOUT_MAX_VALUE
		* (uint64_t)timeout->wrap_counter;

	return (int64_t)deadline;
}