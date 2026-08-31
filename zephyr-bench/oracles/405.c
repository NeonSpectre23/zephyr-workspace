float timeutil_sync_estimate_skew(const struct timeutil_sync_state *tsp)
{
	float rv = 0;

	if ((tsp->base.ref != 0) && (tsp->latest.ref != 0)
	    && (tsp->latest.local > tsp->base.local)) {
		const struct timeutil_sync_config *cfg = tsp->cfg;
		double ref_delta = tsp->latest.ref - tsp->base.ref;
		double local_delta = tsp->latest.local - tsp->base.local;

		rv = ref_delta * cfg->local_Hz / local_delta / cfg->ref_Hz;
	}

	return rv;
}