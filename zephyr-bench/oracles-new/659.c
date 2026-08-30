int64_t pm_policy_next_event_ticks(void)
{
	int64_t ticks = -1;

	K_SPINLOCK(&events_lock) {
		if (next_event == NULL) {
			K_SPINLOCK_BREAK;
		}

		ticks = next_event->uptime_ticks - k_uptime_ticks();

		if (ticks < 0) {
			ticks = 0;
		}
	}

	return ticks;
}