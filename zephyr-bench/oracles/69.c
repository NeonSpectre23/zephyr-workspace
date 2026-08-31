int fs_sync(struct fs_file_t *zfp)
{
	int rc = -EINVAL;

	if (zfp->mp == NULL) {
		return -EBADF;
	}

	CHECKIF(zfp->mp->fs->sync == NULL) {
		return -ENOTSUP;
	}

	rc = zfp->mp->fs->sync(zfp);
	if (rc < 0) {
		LOG_ERR("file sync error (%d)", rc);
	}

	return rc;
}