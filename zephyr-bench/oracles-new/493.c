osStatus_t osEventFlagsDelete(osEventFlagsId_t ef_id)
{
	struct cmsis_rtos_event_cb *events = (struct cmsis_rtos_event_cb *)ef_id;

	if (ef_id == NULL) {
		return osErrorResource;
	}

	if (k_is_in_isr()) {
		return osErrorISR;
	}

	/* The status code "osErrorParameter" (the value of the parameter
	 * ef_id is incorrect) is not supported in Zephyr.
	 */
	if (events->is_cb_dynamic_allocation) {
		k_mem_slab_free(&cmsis_rtos_event_cb_slab, (void *)events);
	}
	return osOK;
}