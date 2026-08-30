ssize_t zms_active_sector_free_space(struct zms_fs *fs)
{
	if (!fs) {
		LOG_ERR("Invalid fs");
		return -EINVAL;
	}

	if (!fs->ready) {
		LOG_ERR("ZMS not initialized");
		return -EACCES;
	}

	return zms_free_space(fs, SECTOR_OFFSET(fs->data_wra), SECTOR_OFFSET(fs->ate_wra));
}