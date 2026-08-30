int uuid_generate_v4(struct uuid *out)
{
	if (out == NULL) {
		return -EINVAL;
	}
	/* Fill the whole UUID struct with a random number */
	sys_rand_get(out->val, UUID_SIZE);
	/* Update version and variant */
	overwrite_uuid_version_and_variant(UUID_V4_VERSION, UUID_V4_VARIANT, out);
	return 0;
}