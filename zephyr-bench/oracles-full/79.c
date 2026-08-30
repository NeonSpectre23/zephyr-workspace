int fs_mount(struct fs_mount_t *mp)
{
	struct fs_mount_t *itr;
	const struct fs_file_system_t *fs;
	sys_dnode_t *node;
	int rc = -EINVAL;
	size_t len = 0;

	/* Do all the mp checks prior to locking the mutex on the file
	 * subsystem.
	 */
	if ((mp == NULL) || (mp->mnt_point == NULL)) {
		LOG_ERR("mount point not initialized!!");
		return -EINVAL;
	}

	if (sys_dnode_is_linked(&mp->node)) {
		LOG_ERR("file system already mounted!!");
		return -EBUSY;
	}

	len = strlen(mp->mnt_point);

	if ((len == 0) || (mp->mnt_point[0] != '/')) {
		LOG_ERR("invalid mount point!!");
		return -EINVAL;
	}

	k_mutex_lock(&mutex, K_FOREVER);

	/* Check if mount point already exists */
	SYS_DLIST_FOR_EACH_NODE(&fs_mnt_list, node) {
		itr = CONTAINER_OF(node, struct fs_mount_t, node);
		/* continue if length does not match */
		if (len != itr->mountp_len) {
			continue;
		}

		CHECKIF(mp->fs_data == itr->fs_data) {
			LOG_ERR("file system already mounted!!");
			rc = -EBUSY;
			goto mount_err;
		}

		if (strncmp(mp->mnt_point, itr->mnt_point, len) == 0) {
			LOG_ERR("mount point already exists!!");
			rc = -EBUSY;
			goto mount_err;
		}
	}

	/* Get file system information */
	fs = fs_type_get(mp->type);
	if (fs == NULL) {
		LOG_ERR("requested file system type not registered!!");
		rc = -ENOENT;
		goto mount_err;
	}

	CHECKIF(fs->mount == NULL) {
		LOG_ERR("fs type %d does not support mounting", mp->type);
		rc = -ENOTSUP;
		goto mount_err;
	}

	if (fs->unmount == NULL) {
		LOG_WRN("mount path %s is not unmountable",
			mp->mnt_point);
	}

	rc = fs->mount(mp);
	if (rc < 0) {
		LOG_ERR("fs mount error (%d)", rc);
		goto mount_err;
	}

	/* Update mount point data and append it to the list */
	mp->mountp_len = len;
	mp->fs = fs;

	sys_dlist_append(&fs_mnt_list, &mp->node);
	LOG_DBG("fs mounted at %s", mp->mnt_point);

mount_err:
	k_mutex_unlock(&mutex);
	return rc;
}