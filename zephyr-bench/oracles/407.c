int timeutil_sync_ref_from_local(const struct timeutil_sync_state *tsp,
				 uint64_t local, uint64_t *refp)
{
	int rv = -EINVAL;

	if ((tsp->skew > 0) && (tsp->base.ref > 0) && (refp != NULL)) {
		const struct timeutil_sync_config *cfg = tsp->cfg;
		int64_t local_delta = local - tsp->base.local;
#ifdef CONFIG_TIMEUTIL_APPLY_SKEW
		/* (x * 1.0) != x for large values of x.
		 * Therefore only apply the multiplication if the skew is not one.
		 */
		if (tsp->skew != 1.0f) {
			local_delta *= (double)tsp->skew;
		}
#endif /* CONFIG_TIMEUTIL_APPLY_SKEW */
		int64_t ref_delta = local_delta * cfg->ref_Hz / cfg->local_Hz;
		int64_t ref_abs = (int64_t)tsp->base.ref + ref_delta;

		if (ref_abs < 0) {
			rv = -ERANGE;
		} else {
			*refp = ref_abs;
			rv = (tsp->skew != 1.0f) ? 1 : 0;
		}
	}

	return rv;
}