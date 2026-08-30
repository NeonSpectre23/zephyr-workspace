static inline int linear_range_group_get_win_index(const struct linear_range *r,
						   size_t r_cnt,
						   int32_t val_min,
						   int32_t val_max,
						   uint16_t *idx)
{
	for (size_t i = 0U; i < r_cnt; i++) {
		if (val_min > linear_range_get_max_value(&r[i])) {
			continue;
		}

		return linear_range_get_win_index(&r[i], val_min, val_max, idx);
	}

	return -EINVAL;
}