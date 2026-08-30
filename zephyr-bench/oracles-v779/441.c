int timeutil_sync_state_set_skew(struct timeutil_sync_state *tsp, float skew,
				 const struct timeutil_sync_instant *base)
{
	int rv = -EINVAL;

	if (skew > 0) {
		tsp->skew = skew;
		if (base != NULL) {
			tsp->base = *base;
			tsp->latest = (struct timeutil_sync_instant){};
		}
		rv = 0;
	}

	return rv;
}