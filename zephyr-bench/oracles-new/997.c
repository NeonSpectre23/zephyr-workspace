int uuid_generate_v5(const struct uuid *ns, const void *data, size_t data_size,
		     struct uuid *out)
{
	uint8_t sha_result[PSA_HASH_LENGTH(PSA_ALG_SHA_1)];
	psa_hash_operation_t hash_operation = PSA_HASH_OPERATION_INIT;
	size_t sha_len;
	psa_status_t status;

	if (out == NULL) {
		return -EINVAL;
	}

	status = psa_hash_setup(&hash_operation, PSA_ALG_SHA_1);
	if (status != PSA_SUCCESS) {
		goto exit;
	}

	status = psa_hash_update(&hash_operation, ns->val, UUID_SIZE);
	if (status != PSA_SUCCESS) {
		goto exit;
	}

	status = psa_hash_update(&hash_operation, data, data_size);
	if (status != PSA_SUCCESS) {
		goto exit;
	}

	status = psa_hash_finish(&hash_operation, sha_result, sizeof(sha_result), &sha_len);
	if (status != PSA_SUCCESS) {
		goto exit;
	}

	/* Store the computed SHA1 in the out struct */
	for (unsigned int i = 0; i < UUID_SIZE; i++) {
		out->val[i] = sha_result[i];
	}
	/* Update version and variant */
	overwrite_uuid_version_and_variant(UUID_V5_VERSION, UUID_V5_VARIANT, out);

exit:
	psa_hash_abort(&hash_operation);

	switch (status) {
	case PSA_SUCCESS:
		return 0;
	case PSA_ERROR_INSUFFICIENT_MEMORY:
		return -ENOMEM;
	case PSA_ERROR_NOT_SUPPORTED:
		return -ENOTSUP;
	default:
		return -EIO;
	}
}