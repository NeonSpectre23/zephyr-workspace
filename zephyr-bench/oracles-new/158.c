ssize_t fs_write(struct fs_file_t *zfp, const void *ptr, size_t size)
{
	int rc = -EINVAL;

	if (zfp->mp == NULL) {
		return -EBADF;
	}

	CHECKIF(zfp->mp->fs->write == NULL) {
		return -ENOTSUP;
	}

	rc = zfp->mp->fs->write(zfp, ptr, size);
	if (rc < 0) {
		LOG_ERR("file write error (%d)", rc);
	}

	return rc;
}