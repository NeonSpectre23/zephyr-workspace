uint32_t osEventFlagsClear(osEventFlagsId_t ef_id, uint32_t flags)
{
	struct cmsis_rtos_event_cb *events = (struct cmsis_rtos_event_cb *)ef_id;
	uint32_t rv;

	if ((ef_id == NULL) || (flags & osFlagsError)) {
		return osFlagsErrorParameter;
	}

	rv = k_event_test(&events->z_event, 0xFFFFFFFF);
	k_event_clear(&events->z_event, flags & rv);

	return rv;
}