int onoff_release(struct onoff_manager *mgr)
{
	bool stop = false;      /* trigger a stop transition */

	k_spinlock_key_t key = k_spin_lock(&mgr->lock);
	uint32_t state = mgr->flags & ONOFF_STATE_MASK;
	int rv = state;

	if (state != ONOFF_STATE_ON) {
		if (state == ONOFF_STATE_ERROR) {
			rv = -EIO;
		} else {
			rv = -ENOTSUP;
		}
		goto out;
	}

	__ASSERT_NO_MSG(mgr->refs > 0);
	mgr->refs -= 1U;
	stop = (mgr->refs == 0);

out:
	if (stop) {
		process_event(mgr, EVT_RECHECK, key);
	} else {
		k_spin_unlock(&mgr->lock, key);
	}

	return rv;
}