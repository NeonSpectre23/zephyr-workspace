osStatus osTimerDelete(osTimerId timer_id)
{
	struct timer_obj *timer = (struct timer_obj *) timer_id;

	if (timer == NULL) {
		return osErrorParameter;
	}

	if (k_is_in_isr()) {
		return osErrorISR;
	}

	if (timer->status == ACTIVE) {
		k_timer_stop(&timer->ztimer);
		timer->status = NOT_ACTIVE;
	}

	k_mem_slab_free(&cmsis_timer_slab, (void *)timer);
	return osOK;
}