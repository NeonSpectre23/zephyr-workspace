int fs_mkfs(int fs_type, uintptr_t dev_id, void *cfg, int flags)
{
	int rc = -EINVAL;
	const struct fs_file_system_t *fs;

	k_mutex_lock(&mutex, K_FOREVER);

	/* Get file system information */
	fs = fs_type_get(fs_type);
	if (fs == NULL) {
		LOG_ERR("fs type %d not registered!!",
				fs_type);
		rc = -ENOENT;
		goto mount_err;
	}

	CHECKIF(fs->mkfs == NULL) {
		LOG_ERR("fs type %d does not support mkfs", fs_type);
		rc = -ENOTSUP;
		goto mount_err;
	}

	rc = fs->mkfs(dev_id, cfg, flags);
	if (rc < 0) {
		LOG_ERR("mkfs error (%d)", rc);
		goto mount_err;
	}

mount_err:
	k_mutex_unlock(&mutex);
	return rc;
}