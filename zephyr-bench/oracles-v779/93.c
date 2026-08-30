int json_mixed_arr_encode_buf(const struct json_mixed_arr_descr *descr,
			      size_t descr_len, void *val,
			      char *buffer, size_t buf_size)
{
	struct appender appender = { .buffer = buffer, .size = buf_size };

	return json_mixed_arr_encode(descr, descr_len, val, append_bytes_to_buf, &appender);
}