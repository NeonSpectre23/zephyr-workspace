osTimerId osTimerCreate(const osTimerDef_t *timer_def, os_timer_type type,
			void *argument)
{
	struct timer_obj *timer;

	if (timer_def == NULL) {
		return NULL;
	}

	if (type != osTimerOnce && type != osTimerPeriodic) {
		return NULL;
	}

	if (k_mem_slab_alloc(&cmsis_timer_slab, (void **)&timer, K_MSEC(100)) == 0) {
		(void)memset(timer, 0, sizeof(struct timer_obj));
	} else {
		return NULL;
	}

	timer->callback_function = timer_def->ptimer;
	timer->arg = argument;
	timer->type = type;
	timer->status = NOT_ACTIVE;

	k_timer_init(&timer->ztimer, zephyr_timer_wrapper, NULL);

	return (osTimerId)timer;
}