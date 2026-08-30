int onoff_cancel(struct onoff_manager *mgr,
		 struct onoff_client *cli)
{
	if ((mgr == NULL) || (cli == NULL)) {
		return -EINVAL;
	}

	int rv = -EALREADY;
	k_spinlock_key_t key = k_spin_lock(&mgr->lock);
	uint32_t state = mgr->flags & ONOFF_STATE_MASK;

	if (sys_slist_find_and_remove(&mgr->clients, &cli->node)) {
		__ASSERT_NO_MSG((state == ONOFF_STATE_TO_ON)
				|| (state == ONOFF_STATE_TO_OFF)
				|| (state == ONOFF_STATE_RESETTING));
		rv = state;
	}

	k_spin_unlock(&mgr->lock, key);

	return rv;
}