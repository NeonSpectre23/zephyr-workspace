int uuid_from_string(const char data[UUID_STR_LEN], struct uuid *out)
{
	if ((data == NULL) || (strlen(data) + 1 != UUID_STR_LEN) || (out == NULL)) {
		return -EINVAL;
	}
	for (unsigned int i = 0; i < UUID_STR_LEN - 1; i++) {
		char char_i = data[i];
		/* Check that hyphens are in the right place */
		if (should_be_hyphen(i)) {
			if (char_i != '-') {
				return -EINVAL;
			}
			continue;
		}
		/* Check if the given input is hexadecimal */
		if (!isxdigit(char_i)) {
			return -EINVAL;
		}
	}

	/* Content parsing */
	unsigned int data_idx = 0U;
	unsigned int out_idx = 0U;

	while (data_idx < UUID_STR_LEN - 1) {
		if (should_be_hyphen(data_idx)) {
			data_idx += 1;
			continue;
		}

		size_t hex2bin_rc =
			hex2bin(&data[data_idx], 2, &out->val[out_idx], UUID_SIZE - out_idx);
		if (hex2bin_rc != 1) {
			return -EINVAL;
		}
		out_idx++;
		data_idx += 2;
	}
	return 0;
}