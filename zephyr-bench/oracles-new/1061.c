int zms_clear(struct zms_fs *fs)
{
	int rc;

	if (!fs) {
		LOG_ERR("Invalid fs");
		return -EINVAL;
	}

	if (!fs->ready) {
		LOG_ERR("zms not initialized");
		return -EACCES;
	}

	k_mutex_lock(&fs->zms_lock, K_FOREVER);

	rc = zms_wipe_partition(fs);

	/* zms needs to be reinitialized after clearing */
	fs->ready = false;

	k_mutex_unlock(&fs->zms_lock);

	return rc;
}