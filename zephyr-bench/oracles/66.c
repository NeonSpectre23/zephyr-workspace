int fs_register(int type, const struct fs_file_system_t *fs)
{
	int rc = 0;

	k_mutex_lock(&mutex, K_FOREVER);

	if (fs_type_get(type) != NULL) {
		rc = -EALREADY;
	} else {
		rc = registry_add(type, fs);
	}

	k_mutex_unlock(&mutex);

	LOG_DBG("fs register %d: %d", type, rc);

	return rc;
}