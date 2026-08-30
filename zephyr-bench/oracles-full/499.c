int timeutil_sync_local_from_ref(const struct timeutil_sync_state *tsp,
				 uint64_t ref, int64_t *localp)
{
	int rv = -EINVAL;

	if ((tsp->skew > 0) && (tsp->base.ref > 0) && (localp != NULL)) {
		const struct timeutil_sync_config *cfg = tsp->cfg;
		int64_t ref_delta = (int64_t)(ref - tsp->base.ref);
		int64_t local_delta = (ref_delta * cfg->local_Hz) / cfg->ref_Hz;
#ifdef CONFIG_TIMEUTIL_APPLY_SKEW
		/* (x / 1.0) != x for large values of x.
		 * Therefore only apply the division if the skew is not one.
		 */
		if (tsp->skew != 1.0f) {
			local_delta /= (double)tsp->skew;
		}
#endif /* CONFIG_TIMEUTIL_APPLY_SKEW */
		int64_t local_abs = (int64_t)tsp->base.local
				    + (int64_t)local_delta;

		*localp = local_abs;
		rv = (tsp->skew != 1.0f) ? 1 : 0;
	}

	return rv;
}