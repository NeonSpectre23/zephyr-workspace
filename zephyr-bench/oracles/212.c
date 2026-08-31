uint32_t net_timeout_evaluate(struct net_timeout *timeout,
			      uint32_t now)
{
	uint32_t elapsed;
	uint32_t last_delay;
	int32_t remains;
	bool wraps;

	/* Time since last evaluation or set. */
	elapsed = now - timeout->timer_start;

	/* The delay used the last time this was evaluated. */
	wraps = (timeout->wrap_counter > 0U);
	last_delay = wraps
		? NET_TIMEOUT_MAX_VALUE
		: timeout->timer_timeout;

	/* Time remaining until completion of the last delay. */
	remains = (int32_t)(last_delay - elapsed);

	/* If the deadline for the next event hasn't been reached yet just
	 * return the remaining time.
	 */
	if (remains > 0) {
		return (uint32_t)remains;
	}

	/* Deadline has been reached.  If we're not wrapping we've completed
	 * the last portion of the full timeout, so return zero to indicate
	 * the timeout has completed.
	 */
	if (!wraps) {
		return 0U;
	}

	/* There's more to do.  We need to update timer_start to correspond to
	 * now, then reduce the remaining time by the elapsed time.  We know
	 * that's at least NET_TIMEOUT_MAX_VALUE, and can apply the
	 * reduction by decrementing the wrap count.
	 */
	timeout->timer_start = now;
	elapsed -= NET_TIMEOUT_MAX_VALUE;
	timeout->wrap_counter -= 1;

	/* The residual elapsed must reduce timer_timeout, which is capped at
	 * NET_TIMEOUT_MAX_VALUE.  But if subtracting would reduce the
	 * counter to zero or go negative we need to reduce the wrap
	 * counter once more and add the residual to the counter, so the
	 * counter remains positive.
	 */
	if (timeout->timer_timeout > elapsed) {
		timeout->timer_timeout -= elapsed;
	} else {
		timeout->timer_timeout += NET_TIMEOUT_MAX_VALUE - elapsed;
		timeout->wrap_counter -= 1U;
	}

	return (timeout->wrap_counter == 0U)
		? timeout->timer_timeout
		: NET_TIMEOUT_MAX_VALUE;
}