int fs_unlink(const char *abs_path)
{
	struct fs_mount_t *mp;
	int rc = -EINVAL;

	if ((abs_path == NULL) || (abs_path[0] != '/') || (abs_path[1] == '\0')) {
		LOG_ERR("invalid file name!!");
		return -EINVAL;
	}

	rc = fs_get_mnt_point(&mp, abs_path, NULL);
	if (rc < 0) {
		LOG_ERR("mount point not found!!");
		return rc;
	}

	if (mp->flags & FS_MOUNT_FLAG_READ_ONLY) {
		return -EROFS;
	}

	CHECKIF(mp->fs->unlink == NULL) {
		return -ENOTSUP;
	}

	rc = mp->fs->unlink(mp, abs_path);
	if (rc < 0) {
		LOG_ERR("failed to unlink path (%d)", rc);
	}

	return rc;
}