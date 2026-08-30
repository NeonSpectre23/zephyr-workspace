int json_obj_encode_buf(const struct json_obj_descr *descr, size_t descr_len,
			const void *val, char *buffer, size_t buf_size)
{
	struct appender appender = { .buffer = buffer, .size = buf_size };

	return json_obj_encode(descr, descr_len, val, append_bytes_to_buf,
			       &appender);
}