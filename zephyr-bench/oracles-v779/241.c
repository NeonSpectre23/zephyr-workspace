int onoff_reset(struct onoff_manager *mgr,
		struct onoff_client *cli)
{
	bool reset = false;
	int rv = validate_args(mgr, cli);

	if ((rv >= 0)
	    && (mgr->transitions->reset == NULL)) {
		rv = -ENOTSUP;
	}

	if (rv < 0) {
		return rv;
	}

	k_spinlock_key_t key = k_spin_lock(&mgr->lock);
	uint32_t state = mgr->flags & ONOFF_STATE_MASK;

	rv = state;

	if ((state & ONOFF_FLAG_ERROR) == 0) {
		rv = -EALREADY;
	} else {
		reset = (state != ONOFF_STATE_RESETTING);
		sys_slist_append(&mgr->clients, &cli->node);
	}

	if (reset) {
		process_event(mgr, EVT_RECHECK, key);
	} else {
		k_spin_unlock(&mgr->lock, key);
	}

	return rv;
}