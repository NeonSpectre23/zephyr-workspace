int onoff_monitor_register(struct onoff_manager *mgr,
			   struct onoff_monitor *mon)
{
	if ((mgr == NULL)
	    || (mon == NULL)
	    || (mon->callback == NULL)) {
		return -EINVAL;
	}

	k_spinlock_key_t key = k_spin_lock(&mgr->lock);

	sys_slist_append(&mgr->monitors, &mon->node);

	k_spin_unlock(&mgr->lock, key);

	return 0;
}