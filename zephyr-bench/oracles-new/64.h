static inline int linear_range_get_value(const struct linear_range *r,
					 uint16_t idx, int32_t *val)
{
	if ((idx < r->min_idx) || (idx > r->max_idx)) {
		return -EINVAL;
	}

	*val = r->min + (int32_t)(r->step * (idx - r->min_idx));

	return 0;
}