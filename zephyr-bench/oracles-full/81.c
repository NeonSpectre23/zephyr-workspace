int fs_opendir(struct fs_dir_t *zdp, const char *abs_path)
{
	struct fs_mount_t *mp;
	int rc = -EINVAL;

	if ((abs_path == NULL) || (abs_path[0] != '/')) {
		LOG_ERR("invalid directory name!!");
		return -EINVAL;
	}

	if (zdp->mp != NULL || zdp->dirp != NULL) {
		return -EBUSY;
	}


	if (strcmp(abs_path, "/") == 0) {
		/* Open VFS root dir, marked by zdp->mp == NULL */
		k_mutex_lock(&mutex, K_FOREVER);

		zdp->mp = NULL;
		zdp->dirp = sys_dlist_peek_head(&fs_mnt_list);

		k_mutex_unlock(&mutex);

		return 0;
	}

	rc = fs_get_mnt_point(&mp, abs_path, NULL);
	if (rc < 0) {
		LOG_ERR("mount point not found!!");
		return rc;
	}

	CHECKIF(mp->fs->opendir == NULL) {
		return -ENOTSUP;
	}

	zdp->mp = mp;
	rc = zdp->mp->fs->opendir(zdp, abs_path);
	if (rc < 0) {
		zdp->mp = NULL;
		zdp->dirp = NULL;
		LOG_ERR("directory open error (%d)", rc);
	}

	return rc;
}