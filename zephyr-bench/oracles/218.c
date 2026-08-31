int onoff_monitor_unregister(struct onoff_manager *mgr,
			     struct onoff_monitor *mon)
{
	int rv = -EINVAL;

	if ((mgr == NULL)
	    || (mon == NULL)) {
		return rv;
	}

	k_spinlock_key_t key = k_spin_lock(&mgr->lock);

	if (sys_slist_find_and_remove(&mgr->monitors, &mon->node)) {
		rv = 0;
	}

	k_spin_unlock(&mgr->lock, key);

	return rv;
}