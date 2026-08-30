int fs_closedir(struct fs_dir_t *zdp)
{
	int rc = -EINVAL;

	if (zdp->mp == NULL) {
		/* VFS root dir */
		zdp->dirp = NULL;
		return 0;
	}

	CHECKIF(zdp->mp->fs->closedir == NULL) {
		return -ENOTSUP;
	}

	rc = zdp->mp->fs->closedir(zdp);
	if (rc < 0) {
		LOG_ERR("directory close error (%d)", rc);
		return rc;
	}

	zdp->mp = NULL;
	zdp->dirp = NULL;
	return rc;
}