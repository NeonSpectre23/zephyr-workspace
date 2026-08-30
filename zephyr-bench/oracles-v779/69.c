int fs_stat(const char *abs_path, struct fs_dirent *entry)
{
	struct fs_mount_t *mp;
	int rc = -EINVAL;
	size_t mp_len;
	size_t path_len;

	if ((abs_path == NULL) || (abs_path[0] != '/')) {
		LOG_ERR("invalid file or dir name!!");
		return -EINVAL;
	}

	path_len = strlen(abs_path);
	if (path_len == 1) {
		/* Stat on the root directory */
		entry->type = FS_DIR_ENTRY_DIR;
		entry->name[0] = '/';
		entry->name[1] = '\0';
		entry->size = 0;
		return 0;
	}

	rc = fs_get_mnt_point(&mp, abs_path, &mp_len);
	if (rc < 0) {
		LOG_ERR("mount point not found!!");
		return rc;
	}

	/* abs_path can have a / at the end, unlike mnt_point. */
	if (path_len - mp_len <= 1U) {
		/* Stat on the mount point itself */
		fs_copy_mnt_point_to_entry(mp, entry);
		return 0;
	}

	CHECKIF(mp->fs->stat == NULL) {
		return -ENOTSUP;
	}

	rc = mp->fs->stat(mp, abs_path, entry);
	if (rc == -ENOENT) {
		/* File doesn't exist, which is a valid stat response */
	} else if (rc < 0) {
		LOG_ERR("failed get file or dir stat (%d)", rc);
	}
	return rc;
}