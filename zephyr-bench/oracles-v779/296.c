osEvent osSignalWait(int32_t signals, uint32_t millisec)
{
	int retval, key;
	osEvent evt = {0};
	uint32_t time_delta_ms, timeout = millisec;
	uint64_t time_stamp_start, hwclk_cycles_delta, time_delta_ns;

	if (k_is_in_isr()) {
		evt.status = osErrorISR;
		return evt;
	}

	/* Check if signals is within the permitted range */
	if ((signals & 0x80000000) || (signals > MAX_VALID_SIGNAL_VAL)) {
		evt.status = osErrorValue;
		return evt;
	}

	osThreadDef_t *thread_def = k_thread_custom_data_get();

	for (;;) {

		time_stamp_start = (uint64_t)k_cycle_get_32();

		switch (millisec) {
		case 0:
			retval = k_poll(thread_def->poll_event, 1, K_NO_WAIT);
			break;
		case osWaitForever:
			retval = k_poll(thread_def->poll_event, 1, K_FOREVER);
			break;
		default:
			retval = k_poll(thread_def->poll_event, 1,
					K_MSEC(timeout));
			break;
		}

		switch (retval) {
		case 0:
			evt.status = osEventSignal;
			break;
		case -EAGAIN:
			if (millisec == 0U) {
				evt.status = osOK;
			} else {
				evt.status = osEventTimeout;
			}
			return evt;
		default:
			evt.status = osErrorValue;
			return evt;
		}

		__ASSERT(thread_def->poll_event->state
				== K_POLL_STATE_SIGNALED,
			"event state not signalled!");
		__ASSERT(thread_def->poll_event->signal->signaled == 1,
			"event signaled is not 1");

		/* Reset the states to facilitate the next trigger */
		thread_def->poll_event->signal->signaled = 0;
		thread_def->poll_event->state = K_POLL_STATE_NOT_READY;

		/* Check if all events we are waiting on have been signalled */
		if ((thread_def->signal_results & signals) == signals) {
			break;
		}

		/* If we need to wait on more signals, we need to adjust the
		 * timeout value accordingly based on the time that has
		 * already elapsed.
		 */
		hwclk_cycles_delta = (uint64_t)k_cycle_get_32() - time_stamp_start;
		time_delta_ns =
			(uint32_t)k_cyc_to_ns_floor64(hwclk_cycles_delta);
		time_delta_ms =	(uint32_t)time_delta_ns/NSEC_PER_MSEC;

		if (timeout > time_delta_ms) {
			timeout -= time_delta_ms;
		} else {
			timeout = 0U;
		}
	}

	evt.value.signals = thread_def->signal_results;

	/* Clear signal flags as the thread is ready now */
	key = irq_lock();
	thread_def->signal_results &= signals ? ~(signals) : 0;
	irq_unlock(key);

	return evt;
}