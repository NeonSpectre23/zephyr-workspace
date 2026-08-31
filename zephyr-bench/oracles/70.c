off_t fs_tell(struct fs_file_t *zfp)
{
	int rc = -ENOTSUP;

	if (zfp->mp == NULL) {
		return -EBADF;
	}

	CHECKIF(zfp->mp->fs->tell == NULL) {
		return -ENOTSUP;
	}

	rc = zfp->mp->fs->tell(zfp);
	if (rc < 0) {
		LOG_ERR("file tell error (%d)", rc);
	}

	return rc;
}