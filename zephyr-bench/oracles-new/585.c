osStatus osTimerStart(osTimerId timer_id, uint32_t millisec)
{
	struct timer_obj *timer = (struct timer_obj *) timer_id;

	if (timer == NULL) {
		return osErrorParameter;
	}

	if (k_is_in_isr()) {
		return osErrorISR;
	}

	if (timer->type == osTimerOnce) {
		k_timer_start(&timer->ztimer, K_MSEC(millisec), K_NO_WAIT);
	} else if (timer->type == osTimerPeriodic) {
		k_timer_start(&timer->ztimer, K_MSEC(millisec),
			      K_MSEC(millisec));
	}

	timer->status = ACTIVE;
	return osOK;
}