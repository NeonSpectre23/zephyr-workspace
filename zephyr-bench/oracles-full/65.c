int flash_area_check_int_sha256(const struct flash_area *fa,
				const struct flash_area_check *fac)
{
	unsigned char hash[PSA_HASH_LENGTH(PSA_ALG_SHA_256)];
	psa_hash_operation_t hash_ctx;
	size_t hash_len;
	int to_read;
	int pos;
	int rc;

	if (fa == NULL || fac == NULL || fac->match == NULL ||
	    fac->rbuf == NULL || fac->clen == 0 || fac->rblen == 0) {
		return -EINVAL;
	}

	if (!is_in_flash_area_bounds(fa, fac->off, fac->clen)) {
		return -EINVAL;
	}

	hash_ctx = psa_hash_operation_init();
	rc = psa_hash_setup(&hash_ctx, PSA_ALG_SHA_256);
	if (rc != PSA_SUCCESS) {
		return -ESRCH;
	}

	to_read = fac->rblen;

	for (pos = 0; pos < fac->clen; pos += to_read) {
		if (pos + to_read > fac->clen) {
			to_read = fac->clen - pos;
		}

		rc = flash_read(fa->fa_dev, (fa->fa_off + fac->off + pos),
				fac->rbuf, to_read);
		if (rc != 0) {
			goto error;
		}

		rc = psa_hash_update(&hash_ctx, fac->rbuf, to_read);
		if (rc != PSA_SUCCESS) {
			rc = -ESRCH;
			goto error;
		}
	}

	rc = psa_hash_finish(&hash_ctx, hash, sizeof(hash), &hash_len);
	if (rc != PSA_SUCCESS) {
		rc = -ESRCH;
		goto error;
	}

	if (memcmp(hash, fac->match, sizeof(hash))) {
		/* The operation has already been terminated. */
		return -EILSEQ;
	}

error:
	psa_hash_abort(&hash_ctx);
	return rc;
}