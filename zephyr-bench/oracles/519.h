static inline int linear_range_group_get_value(const struct linear_range *r,
					       size_t r_cnt, uint16_t idx,
					       int32_t *val)
{
	int ret = -EINVAL;

	for (size_t i = 0U; (ret != 0) && (i < r_cnt); i++) {
		ret = linear_range_get_value(&r[i], idx, val);
	}

	return ret;
}