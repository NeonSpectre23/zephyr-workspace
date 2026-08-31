static inline int onoff_cancel_or_release(struct onoff_manager *mgr,
					  struct onoff_client *cli)
{
	int rv = onoff_cancel(mgr, cli);

	if (rv == -EALREADY) {
		rv = onoff_release(mgr);
	}
	return rv;
}