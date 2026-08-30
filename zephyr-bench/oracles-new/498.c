uint32_t osEventFlagsWait(osEventFlagsId_t ef_id, uint32_t flags, uint32_t options,
			  uint32_t timeout)
{
	struct cmsis_rtos_event_cb *events = (struct cmsis_rtos_event_cb *)ef_id;
	uint32_t sub_opt = options & (osFlagsWaitAll | osFlagsNoClear);
	uint32_t rv;
	k_timeout_t event_timeout;

	/*
	 * Return unknown error if called from ISR with a non-zero timeout
	 * or if flags is zero.
	 */
	if (((timeout > 0U) && k_is_in_isr()) || (flags == 0U)) {
		return osFlagsErrorUnknown;
	}

	if ((ef_id == NULL) || (flags & osFlagsError)) {
		return osFlagsErrorParameter;
	}

	if (timeout == osWaitForever) {
		event_timeout = K_FOREVER;
	} else if (timeout == 0U) {
		event_timeout = K_NO_WAIT;
	} else {
		event_timeout = K_TICKS(timeout);
	}

	switch (sub_opt) {
	case osFlagsWaitAll | osFlagsNoClear:
		rv = k_event_wait_all(&events->z_event, flags, false, event_timeout);
		break;
	case osFlagsWaitAll:
		rv = k_event_wait_all_safe(&events->z_event, flags, false, event_timeout);
		break;
	case osFlagsNoClear:
		rv = k_event_wait(&events->z_event, flags, false, event_timeout);
		break;
	case 0:
		rv = k_event_wait_safe(&events->z_event, flags, false, event_timeout);
		break;
	default:
		__ASSERT_NO_MSG(0);
	}

	if (rv != 0U) {
		return rv;
	}

	return (timeout == 0U) ? osFlagsErrorResource : osFlagsErrorTimeout;
}