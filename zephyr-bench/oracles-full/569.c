int uuid_to_base64(const struct uuid *data, char out[UUID_BASE64_LEN])
{
	if (out == NULL) {
		return -EINVAL;
	}

	size_t olen = 0;

	base64_encode(out, UUID_BASE64_LEN, &olen, data->val, UUID_SIZE);
	return 0;
}