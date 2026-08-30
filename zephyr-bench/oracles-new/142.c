int fs_open(struct fs_file_t *zfp, const char *file_name, fs_mode_t flags)
{
	struct fs_mount_t *mp;
	int rc = -EINVAL;
	bool truncate_file = false;

	if ((file_name == NULL) || (file_name[0] != '/') || (file_name[1] == '\0')) {
		LOG_ERR("invalid file name!!");
		return -EINVAL;
	}

	if (zfp->mp != NULL) {
		return -EBUSY;
	}

	rc = fs_get_mnt_point(&mp, file_name, NULL);
	if (rc < 0) {
		LOG_ERR("mount point not found!!");
		return rc;
	}

	if (((mp->flags & FS_MOUNT_FLAG_READ_ONLY) != 0) &&
	    (flags & FS_O_CREATE || flags & FS_O_WRITE)) {
		return -EROFS;
	}

	CHECKIF(mp->fs->open == NULL) {
		return -ENOTSUP;
	}

	if ((flags & FS_O_TRUNC) != 0) {
		if ((flags & FS_O_WRITE) == 0) {
			/** Truncate not allowed when file is not opened for write */
			LOG_ERR("file should be opened for write to truncate!!");
			return -EACCES;
		}
		CHECKIF(mp->fs->truncate == NULL) {
			LOG_ERR("file truncation not supported!!");
			return -ENOTSUP;
		}
		truncate_file = true;
	}

	zfp->mp = mp;
	rc = mp->fs->open(zfp, file_name, flags);
	if (rc < 0) {
		LOG_ERR("file open error (%d)", rc);
		zfp->mp = NULL;
		return rc;
	}

	/* Copy flags to zfp for use with other fs_ API calls */
	zfp->flags = flags;

	if (truncate_file) {
		/* Truncate the opened file to 0 length */
		rc = mp->fs->truncate(zfp, 0);
		if (rc < 0) {
			LOG_ERR("file truncation failed (%d)", rc);
			zfp->mp = NULL;
			return rc;
		}
	}

	return rc;
}