int fs_statvfs(const char *abs_path, struct fs_statvfs *stat)
{
	struct fs_mount_t *mp;
	int rc;

	if ((abs_path == NULL) || (abs_path[0] != '/') || (abs_path[1] == '\0')) {
		LOG_ERR("invalid file or dir name!!");
		return -EINVAL;
	}

	rc = fs_get_mnt_point(&mp, abs_path, NULL);
	if (rc < 0) {
		LOG_ERR("mount point not found!!");
		return rc;
	}

	CHECKIF(mp->fs->statvfs == NULL) {
		return -ENOTSUP;
	}

	rc = mp->fs->statvfs(mp, abs_path, stat);
	if (rc < 0) {
		LOG_ERR("failed get file or dir stat (%d)", rc);
	}

	return rc;
}