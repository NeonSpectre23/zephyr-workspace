int json_arr_parse(char *payload, size_t len,
		   const struct json_obj_descr *descr, void *val)
{
	struct json_obj arr;
	int ret;

	ret = arr_init(&arr, payload, len);
	if (ret < 0) {
		return ret;
	}

	void *ptr = (char *)val + descr->offset;

	return arr_parse(&arr, descr->array.element_descr,
			 descr->array.n_elements, ptr, val);
}