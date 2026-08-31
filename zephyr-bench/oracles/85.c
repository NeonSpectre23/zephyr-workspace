int json_arr_encode_buf(const struct json_obj_descr *descr, const void *val,
			char *buffer, size_t buf_size)
{
	struct appender appender = { .buffer = buffer, .size = buf_size };

	return json_arr_encode(descr, val, append_bytes_to_buf, &appender);
}