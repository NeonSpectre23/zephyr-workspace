static inline uint32_t linear_range_values_count(const struct linear_range *r)
{
	return r->max_idx - r->min_idx + 1U;
}