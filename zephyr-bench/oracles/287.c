uint32_t osTimerIsRunning(osTimerId_t timer_id)
{
	struct cmsis_rtos_timer_cb *timer = (struct cmsis_rtos_timer_cb *)timer_id;

	if (k_is_in_isr() || (timer == NULL)) {
		return 0;
	}

	return !(!(k_timer_remaining_get(&timer->z_timer)));
}