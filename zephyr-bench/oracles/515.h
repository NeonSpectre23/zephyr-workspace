static inline int32_t linear_range_get_max_value(const struct linear_range *r)
{
	return r->min + (int32_t)(r->step * (r->max_idx - r->min_idx));
}