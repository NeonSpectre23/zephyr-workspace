const char *osEventFlagsGetName(osEventFlagsId_t ef_id)
{
	struct cmsis_rtos_event_cb *events = (struct cmsis_rtos_event_cb *)ef_id;

	if (events == NULL) {
		return NULL;
	}
	return events->name;
}