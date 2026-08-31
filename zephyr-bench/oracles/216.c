int onoff_manager_init(struct onoff_manager *mgr,
		       const struct onoff_transitions *transitions)
{
	if ((mgr == NULL)
	    || (transitions == NULL)
	    || (transitions->start == NULL)
	    || (transitions->stop == NULL)) {
		return -EINVAL;
	}

	*mgr = (struct onoff_manager)ONOFF_MANAGER_INITIALIZER(transitions);

	return 0;
}