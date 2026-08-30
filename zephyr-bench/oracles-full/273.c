int onoff_request(struct onoff_manager *mgr,
		  struct onoff_client *cli)
{
	bool add_client = false;        /* add client to pending list */
	bool start = false;             /* trigger a start transition */
	bool notify = false;            /* do client notification */
	int rv = validate_args(mgr, cli);

	if (rv < 0) {
		return rv;
	}

	k_spinlock_key_t key = k_spin_lock(&mgr->lock);
	uint32_t state = mgr->flags & ONOFF_STATE_MASK;

	/* Reject if this would overflow the reference count. */
	if (mgr->refs == SERVICE_REFS_MAX) {
		rv = -EAGAIN;
		goto out;
	}

	rv = state;
	if (state == ONOFF_STATE_ON) {
		/* Increment reference count, notify in exit */
		notify = true;
		mgr->refs += 1U;
	} else if ((state == ONOFF_STATE_OFF)
		   || (state == ONOFF_STATE_TO_OFF)
		   || (state == ONOFF_STATE_TO_ON)) {
		/* Start if OFF, queue client */
		start = (state == ONOFF_STATE_OFF);
		add_client = true;
	} else if (state == ONOFF_STATE_RESETTING) {
		rv = -ENOTSUP;
	} else {
		__ASSERT_NO_MSG(state == ONOFF_STATE_ERROR);
		rv = -EIO;
	}

out:
	if (add_client) {
		sys_slist_append(&mgr->clients, &cli->node);
	}

	if (start) {
		process_event(mgr, EVT_RECHECK, key);
	} else {
		k_spin_unlock(&mgr->lock, key);

		if (notify) {
			notify_one(mgr, cli, state, 0);
		}
	}

	return rv;
}