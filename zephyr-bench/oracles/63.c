ssize_t fs_read(struct fs_file_t *zfp, void *ptr, size_t size)
{
	int rc = -EINVAL;

	if (zfp->mp == NULL) {
		return -EBADF;
	}

	CHECKIF(zfp->mp->fs->read == NULL) {
		return -ENOTSUP;
	}

	rc = zfp->mp->fs->read(zfp, ptr, size);
	if (rc < 0) {
		LOG_ERR("file read error (%d)", rc);
	}

	return rc;
}