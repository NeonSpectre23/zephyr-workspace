void pm_policy_event_update(struct pm_policy_event *evt, int64_t uptime_ticks)
{
	__ASSERT_NO_MSG(evt != NULL);

	K_SPINLOCK(&events_lock) {
		if (!event_is_registered_locked(evt)) {
			K_SPINLOCK_BREAK;
		}

		evt->uptime_ticks = uptime_ticks;
		update_next_event();
	}
}