void pm_policy_event_register(struct pm_policy_event *evt, int64_t uptime_ticks)
{
	__ASSERT_NO_MSG(evt != NULL);

	K_SPINLOCK(&events_lock) {
		bool registered = event_is_registered_locked(evt);

		/*
		 * Protect against list corruption on accidental double registration.
		 * Re-registering an already registered event behaves like an update.
		 */
		evt->uptime_ticks = uptime_ticks;
		if (!registered) {
			sys_slist_append(&events_list, &evt->node);
		}
		update_next_event();
	}
}