int onoff_sync_finalize(struct onoff_sync_service *srv,
			k_spinlock_key_t key,
			struct onoff_client *cli,
			int res,
			bool on)
{
	uint32_t state = ONOFF_STATE_ON;

	/* Clear errors visible when locked.  If they are to be
	 * preserved the caller must finalize with the previous
	 * error code.
	 */
	if (srv->count < 0) {
		srv->count = 0;
	}
	if (res < 0) {
		srv->count = res;
		state = ONOFF_STATE_ERROR;
	} else if (on) {
		srv->count += 1;
	} else {
		srv->count -= 1;
		/* state would be either off or on, but since
		 * callbacks are used only when turning on don't
		 * bother changing it.
		 */
	}

	int rv = srv->count;

	k_spin_unlock(&srv->lock, key);

	if (cli != NULL) {
		/* Detect service mis-use: onoff does not callback on transition
		 * to off, so no client should have been passed.
		 */
		__ASSERT_NO_MSG(on);
		notify_one(NULL, cli, state, res);
	}

	return rv;
}