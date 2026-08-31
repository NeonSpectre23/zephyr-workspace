uint32_t osEventFlagsGet(osEventFlagsId_t ef_id)
{
	struct cmsis_rtos_event_cb *events = (struct cmsis_rtos_event_cb *)ef_id;

	if (ef_id == NULL) {
		return 0;
	}

	return k_event_test(&events->z_event, 0xFFFFFFFF);
}