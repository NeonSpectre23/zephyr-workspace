int uuid_to_buffer(const struct uuid *data, uint8_t out[UUID_SIZE])
{
	if (out == NULL) {
		return -EINVAL;
	}
	memcpy(out, data->val, UUID_SIZE);
	return 0;
}