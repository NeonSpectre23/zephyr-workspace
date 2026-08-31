int uuid_from_buffer(const uint8_t data[UUID_SIZE], struct uuid *out)
{
	if ((data == NULL) || (out == NULL)) {
		return -EINVAL;
	}
	memcpy(out->val, data, UUID_SIZE);
	return 0;
}