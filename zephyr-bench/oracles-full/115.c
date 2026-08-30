ssize_t json_calc_mixed_arr_len(const struct json_mixed_arr_descr *descr,
				size_t descr_len, void *val)
{
	ssize_t total = 0;
	int ret;

	ret = json_mixed_arr_encode(descr, descr_len, val, measure_bytes, &total);
	if (ret < 0) {
		return ret;
	}

	return total;
}