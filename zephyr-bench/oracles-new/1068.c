int zms_sector_use_next(struct zms_fs *fs)
{
	int ret;

	if (!fs) {
		LOG_ERR("Invalid fs");
		return -EINVAL;
	}

	if (!fs->ready) {
		LOG_ERR("ZMS not initialized");
		return -EACCES;
	}

	k_mutex_lock(&fs->zms_lock, K_FOREVER);

	ret = zms_sector_close(fs);
	if (ret != 0) {
		goto end;
	}

	ret = zms_gc(fs);

end:
	k_mutex_unlock(&fs->zms_lock);
	return ret;
}