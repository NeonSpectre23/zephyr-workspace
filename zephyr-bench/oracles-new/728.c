int settings_save_subtree_or_single_without_modification(const char *name,
							 bool save_if_subtree,
							 bool save_if_single_setting)
{
	int rc;
	int value_size;
	uint8_t read_buffer[CONFIG_SETTINGS_SAVE_SINGLE_SUBTREE_WITHOUT_MODIFICATION_VALUE_SIZE];
	const char *next = NULL;
	struct settings_handler_static *handler;

	if (save_if_subtree == false && save_if_single_setting == false) {
		return -EINVAL;
	}

	handler = settings_parse_and_lookup(name, &next);

	if (next == NULL) {
		/* This is a subtree of settings, bail if user did not request saving it. */
		if (save_if_subtree == false) {
			return -EPERM;
		}

		return settings_save_subtree(name);
	} else if (save_if_single_setting == false) {
		return -EPERM;
	}

	/*
	 * For single settings, we need to be able to retrieve the value of the setting, if this
	 * is not supported then single saving cannot be done with this key.
	 */
	if (handler->h_get == NULL) {
		return -ENOSYS;
	}

	settings_lock_take();

	/*
	 * Settings does not support getting the size of a setting, therefore attempt to read the
	 * full buffer size, if that returns that amount of data then we must abort as we cannot
	 * get the full data with this buffer and would save a truncated value.
	 */
	value_size = handler->h_get(next, read_buffer, sizeof(read_buffer));

	if (value_size < 0) {
		rc = value_size;
		goto exit;
	} else if (value_size == sizeof(read_buffer)) {
		rc = -EDOM;
		goto exit;
	}

	rc = settings_save_one(name, read_buffer, value_size);

	/*
	 * Caller just needs to know that it was successful, not the length of the data that was
	 * saved.
	 */
	if (rc >= 0) {
		rc = 0;
	} else if (rc < 0) {
		LOG_ERR("Saving single setting '%s' of length %d failed: %d", name, value_size,
			rc);
	}

exit:
	settings_lock_release();

	return rc;
}