int64_t json_obj_parse(char *payload, size_t len,
		       const struct json_obj_descr *descr, size_t descr_len,
		       void *val)
{
	struct json_obj obj;
	int64_t ret;

	__ASSERT_NO_MSG(descr_len < (sizeof(ret) * CHAR_BIT - 1));

	ret = obj_init(&obj, payload, len);
	if (ret < 0) {
		return ret;
	}

	return obj_parse(&obj, descr, descr_len, val);
}