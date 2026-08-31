void pm_policy_event_unregister(struct pm_policy_event *evt)
{
	__ASSERT_NO_MSG(evt != NULL);

	K_SPINLOCK(&events_lock) {
		if (!event_is_registered_locked(evt)) {
			K_SPINLOCK_BREAK;
		}

		(void)sys_slist_find_and_remove(&events_list, &evt->node);
		update_next_event();
	}
}