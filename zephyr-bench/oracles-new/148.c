int fs_rename(const char *from, const char *to)
{
	struct fs_mount_t *mp;
	size_t match_len;
	int rc = -EINVAL;

	if ((from == NULL) || (from[0] != '/') || (from[1] == '\0') ||
			(to == NULL) || (to[0] != '/') || (to[1] == '\0')) {
		LOG_ERR("invalid file name!!");
		return -EINVAL;
	}

	rc = fs_get_mnt_point(&mp, from, &match_len);
	if (rc < 0) {
		LOG_ERR("mount point not found!!");
		return rc;
	}

	if (mp->flags & FS_MOUNT_FLAG_READ_ONLY) {
		return -EROFS;
	}

	/* Make sure both files are mounted on the same path */
	if (strncmp(from, to, match_len) != 0) {
		LOG_ERR("mount point not same!!");
		return -EINVAL;
	}

	CHECKIF(mp->fs->rename == NULL) {
		return -ENOTSUP;
	}

	rc = mp->fs->rename(mp, from, to);
	if (rc < 0) {
		LOG_ERR("failed to rename file or dir (%d)", rc);
	}

	return rc;
}