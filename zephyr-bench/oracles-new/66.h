static inline int linear_range_group_get_index(const struct linear_range *r,
					       size_t r_cnt, int32_t val,
					       uint16_t *idx)
{
	for (size_t i = 0U; i < r_cnt; i++) {
		if ((val > linear_range_get_max_value(&r[i])) &&
		    (i < (r_cnt - 1U))) {
			continue;
		}

		return linear_range_get_index(&r[i], val, idx);
	}

	return -EINVAL;
}