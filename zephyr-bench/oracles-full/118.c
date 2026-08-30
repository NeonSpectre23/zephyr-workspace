int json_mixed_arr_parse(char *json, size_t len,
			 const struct json_mixed_arr_descr *descr,
			 size_t descr_len, void *val)
{
	struct json_obj arr;
	int ret;

	ret = arr_init(&arr, json, len);
	if (ret < 0) {
		return ret;
	}

	return mixed_arr_parse(&arr, descr, descr_len, val);
}