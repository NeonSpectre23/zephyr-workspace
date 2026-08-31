int fs_unmount(struct fs_mount_t *mp)
{
	int rc = -EINVAL;

	if (mp == NULL) {
		return rc;
	}

	k_mutex_lock(&mutex, K_FOREVER);

	if (!sys_dnode_is_linked(&mp->node)) {
		LOG_ERR("fs not mounted (mp == %p)", mp);
		goto unmount_err;
	}

	CHECKIF(mp->fs->unmount == NULL) {
		LOG_ERR("fs unmount not supported!!");
		rc = -ENOTSUP;
		goto unmount_err;
	}

	rc = mp->fs->unmount(mp);
	if (rc < 0) {
		LOG_ERR("fs unmount error (%d)", rc);
		goto unmount_err;
	}

	/* remove mount node from the list */
	sys_dlist_remove(&mp->node);
	LOG_DBG("fs unmounted from %s", mp->mnt_point);

unmount_err:
	k_mutex_unlock(&mutex);
	return rc;
}