static inline int linear_range_get_index(const struct linear_range *r,
					 int32_t val, uint16_t *idx)
{
	if (val < r->min) {
		*idx = r->min_idx;
		return -ERANGE;
	}

	if (val > linear_range_get_max_value(r)) {
		*idx = r->max_idx;
		return -ERANGE;
	}

	if (r->step == 0U) {
		*idx = r->min_idx;
	} else {
		*idx = r->min_idx + DIV_ROUND_UP((uint32_t)(val - r->min),
						 r->step);
	}

	return 0;
}