static inline int linear_range_get_win_index(const struct linear_range *r,
					     int32_t val_min, int32_t val_max,
					     uint16_t *idx)
{
	int32_t r_max = linear_range_get_max_value(r);

	if ((val_max < r->min) || (val_min > r_max)) {
		return -EINVAL;
	}

	if (val_min < r->min) {
		*idx = r->min_idx;
		return -ERANGE;
	}

	if (val_max > r_max) {
		*idx = r->max_idx;
		return -ERANGE;
	}

	if (r->step == 0U) {
		*idx = r->min_idx;
		return 0;
	}

	*idx = r->min_idx + DIV_ROUND_UP((uint32_t)(val_min - r->min), r->step);
	if ((r->min + r->step * (*idx - r->min_idx)) > val_max) {
		return -EINVAL;
	}

	return 0;
}