int fs_readdir(struct fs_dir_t *zdp, struct fs_dirent *entry)
{
	if (zdp->mp) {
		/* Delegate to mounted filesystem */
		int rc = -EINVAL;

		CHECKIF(zdp->mp->fs->readdir == NULL) {
			return  -ENOTSUP;
		}

		/* Loop until error or not special directory */
		while (true) {
			rc = zdp->mp->fs->readdir(zdp, entry);
			if (rc < 0) {
				break;
			}
			if (entry->name[0] == 0) {
				break;
			}
			if (entry->type != FS_DIR_ENTRY_DIR) {
				break;
			}
			if ((strcmp(entry->name, ".") != 0)
			    && (strcmp(entry->name, "..") != 0)) {
				break;
			}
		}
		if (rc < 0) {
			LOG_ERR("directory read error (%d)", rc);
		}

		return rc;
	}

	/* VFS root dir */
	if (zdp->dirp == NULL) {
		/* No more entries */
		entry->name[0] = 0;
		return 0;
	}

	/* Find the current and next entries in the mount point dlist */
	sys_dnode_t *node, *next = NULL;
	bool found = false;

	k_mutex_lock(&mutex, K_FOREVER);

	SYS_DLIST_FOR_EACH_NODE(&fs_mnt_list, node) {
		if (node == zdp->dirp) {
			found = true;

			/* Pull info from current entry */
			struct fs_mount_t *mnt;

			mnt = CONTAINER_OF(node, struct fs_mount_t, node);

			fs_copy_mnt_point_to_entry(mnt, entry);

			/* Save pointer to the next one, for later */
			next = sys_dlist_peek_next(&fs_mnt_list, node);
			break;
		}
	}

	k_mutex_unlock(&mutex);

	if (!found) {
		/* Current entry must have been removed before this
		 * call to readdir -- return an error
		 */
		return -ENOENT;
	}

	zdp->dirp = next;
	return 0;
}