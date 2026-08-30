const char *osTimerGetName(osTimerId_t timer_id)
{
	struct cmsis_rtos_timer_cb *timer = (struct cmsis_rtos_timer_cb *)timer_id;

	if (timer == NULL) {
		return NULL;
	}
	return timer->name;
}