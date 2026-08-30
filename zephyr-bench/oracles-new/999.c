int uuid_to_base64url(const struct uuid *data, char out[UUID_BASE64URL_LEN])
{
	if (out == NULL) {
		return -EINVAL;
	}

	/* Convert UUID to RFC 3548/4648 base 64 notation */
	size_t olen = 0;
	char uuid_base64[UUID_BASE64_LEN] = {0};

	base64_encode(uuid_base64, UUID_BASE64_LEN, &olen, data->val, UUID_SIZE);
	/* Convert UUID to RFC 4648 sec. 5 URL and filename safe base 64 notation */
	for (unsigned int i = 0; i < UUID_BASE64URL_LEN - 1; i++) {
		if (uuid_base64[i] == '+') {
			uuid_base64[i] = '-';
		}
		if (uuid_base64[i] == '/') {
			uuid_base64[i] = '_';
		}
	}
	memcpy(out, uuid_base64, UUID_BASE64URL_LEN - 1);
	out[UUID_BASE64URL_LEN - 1] = 0;
	return 0;
}