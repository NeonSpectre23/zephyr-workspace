ssize_t json_calc_encoded_len(const struct json_obj_descr *descr,
			      size_t descr_len, const void *val)
{
	ssize_t total = 0;
	int ret;

	ret = json_obj_encode(descr, descr_len, val, measure_bytes, &total);
	if (ret < 0) {
		return ret;
	}

	return total;
}