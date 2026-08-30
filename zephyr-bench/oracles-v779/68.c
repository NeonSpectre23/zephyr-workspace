int fs_seek(struct fs_file_t *zfp, off_t offset, int whence)
{
	int rc = -ENOTSUP;

	if (zfp->mp == NULL) {
		return -EBADF;
	}

	CHECKIF(zfp->mp->fs->lseek == NULL) {
		return -ENOTSUP;
	}

	rc = zfp->mp->fs->lseek(zfp, offset, whence);
	if (rc < 0) {
		LOG_ERR("file seek error (%d)", rc);
	}

	return rc;
}