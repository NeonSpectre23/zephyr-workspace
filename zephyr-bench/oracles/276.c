uint32_t osThreadFlagsWait(uint32_t flags, uint32_t options, uint32_t timeout)
{
	struct cmsis_rtos_thread_cb *tid;
	int retval, key;
	uint32_t sig;
	uint32_t time_delta_ms, timeout_ms = k_ticks_to_ms_floor64(timeout);
	uint64_t time_stamp_start, hwclk_cycles_delta, time_delta_ns;

	if (k_is_in_isr()) {
		return osFlagsErrorUnknown;
	}

	if (flags & 0x80000000) {
		return osFlagsErrorParameter;
	}

	tid = (struct cmsis_rtos_thread_cb *)osThreadGetId();
	if (tid == NULL) {
		return osFlagsErrorUnknown;
	}

	for (;;) {

		time_stamp_start = (uint64_t)k_cycle_get_32();

		sig = tid->signal_results & flags;

		if (options & osFlagsWaitAll) {
			/* Check if all events we are waiting on have
			 * been signalled
			 */
			if (sig == flags) {
				break;
			}
		} else {
			/* Check if any events we are waiting on have
			 * been signalled
			 */
			if (sig != 0) {
				break;
			}
		}

		switch (timeout) {
		case 0:
			retval = k_poll(&tid->poll_event, 1, K_NO_WAIT);
			break;
		case osWaitForever:
			retval = k_poll(&tid->poll_event, 1, K_FOREVER);
			break;
		default:
			retval = k_poll(&tid->poll_event, 1, K_MSEC(timeout_ms));
			break;
		}

		switch (retval) {
		case 0:
			break;
		case -EAGAIN:
			return osFlagsErrorTimeout;
		default:
			return osFlagsErrorUnknown;
		}

		__ASSERT(tid->poll_event.state == K_POLL_STATE_SIGNALED,
			 "event state not signalled!");
		__ASSERT(tid->poll_event.signal->signaled == 1U, "event signaled is not 1");

		/* Reset the states to facilitate the next trigger */
		tid->poll_event.signal->signaled = 0U;
		tid->poll_event.state = K_POLL_STATE_NOT_READY;

		/* If we need to wait on more signals, we need to
		 * adjust the timeout value accordingly based on
		 * the time that has already elapsed.
		 */
		hwclk_cycles_delta = (uint64_t)k_cycle_get_32() - time_stamp_start;

		time_delta_ns = (uint32_t)k_cyc_to_ns_floor64(hwclk_cycles_delta);

		time_delta_ms = (uint32_t)time_delta_ns / NSEC_PER_MSEC;

		if (timeout_ms > time_delta_ms) {
			timeout_ms -= time_delta_ms;
		} else {
			timeout_ms = 0U;
		}
	}

	if (!(options & osFlagsNoClear)) {
		/* Clear signal flags as the thread is ready now */
		key = irq_lock();
		tid->signal_results &= ~(sig);
		irq_unlock(key);
	}

	return sig;
}