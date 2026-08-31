static inline uint32_t linear_range_group_values_count(
	const struct linear_range *r, size_t r_cnt)
{
	uint32_t values = 0U;

	for (size_t i = 0U; i < r_cnt; i++) {
		values += linear_range_values_count(&r[i]);
	}

	return values;
}