int timeutil_sync_state_update(struct timeutil_sync_state *tsp,
			       const struct timeutil_sync_instant *inst)
{
	int rv = -EINVAL;

	if (((tsp->base.ref == 0) && (inst->ref > 0))
	    || ((inst->ref > tsp->base.ref)
		&& (inst->local > tsp->base.local))) {
		if (tsp->base.ref == 0) {
			tsp->base = *inst;
			tsp->latest = (struct timeutil_sync_instant){};
			tsp->skew = 1.0f;
			rv = 0;
		} else {
			tsp->latest = *inst;
			rv = 1;
		}
	}

	return rv;
}