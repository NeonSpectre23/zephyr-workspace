int uuid_to_string(const struct uuid *data, char out[UUID_STR_LEN])
{
	if (out == NULL) {
		return -EINVAL;
	}
	snprintf(out, UUID_STR_LEN,
		 "%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x",
		 data->val[0], data->val[1], data->val[2], data->val[3], data->val[4], data->val[5],
		 data->val[6], data->val[7], data->val[8], data->val[9], data->val[10],
		 data->val[11], data->val[12], data->val[13], data->val[14], data->val[15]);
	return 0;
}