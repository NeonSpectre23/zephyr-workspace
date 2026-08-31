int fs_truncate(struct fs_file_t *zfp, off_t length)
{
	int rc = -EINVAL;

	if (zfp->mp == NULL) {
		return -EBADF;
	}

	CHECKIF(zfp->mp->fs->truncate == NULL) {
		return -ENOTSUP;
	}

	rc = zfp->mp->fs->truncate(zfp, length);
	if (rc < 0) {
		LOG_ERR("file truncate error (%d)", rc);
	}

	return rc;
}