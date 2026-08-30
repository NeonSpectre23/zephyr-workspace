int bindesc_find_str(struct bindesc_handle *handle, uint16_t id, const char **result)
{
	struct find_user_data data = {
		.tag = BINDESC_TAG(STR, id),
	};

	if (!bindesc_foreach(handle, find_callback, &data)) {
		LOG_WRN("The requested descriptor was not found");
		return -ENOENT;
	}
	*result = (char *)data.result;
	return 0;
}