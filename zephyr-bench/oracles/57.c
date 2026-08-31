int fs_close(struct fs_file_t *zfp)
{
	int rc = -EINVAL;

	if (zfp->mp == NULL) {
		return 0;
	}

	CHECKIF(zfp->mp->fs->close == NULL) {
		return -ENOTSUP;
	}

	rc = zfp->mp->fs->close(zfp);
	if (rc < 0) {
		LOG_ERR("file close error (%d)", rc);
		return rc;
	}

	zfp->mp = NULL;

	return rc;
}