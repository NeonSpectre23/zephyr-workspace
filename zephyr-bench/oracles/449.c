int uuid_copy(const struct uuid *data, struct uuid *out)
{
	if (out == NULL) {
		return -EINVAL;
	}
	memcpy(out->val, data->val, UUID_SIZE);
	return 0;
}