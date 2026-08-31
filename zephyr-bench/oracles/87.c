ssize_t json_calc_encoded_arr_len(const struct json_obj_descr *descr,
				  const void *val)
{
	ssize_t total = 0;
	int ret;

	ret = json_arr_encode(descr, val, measure_bytes, &total);
	if (ret < 0) {
		return ret;
	}

	return total;
}