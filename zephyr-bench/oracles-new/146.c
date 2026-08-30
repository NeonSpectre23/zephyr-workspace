int fs_readmount(int *index, const char **name)
{
	sys_dnode_t *node;
	int rc = -ENOENT;
	int cnt = 0;
	struct fs_mount_t *itr = NULL;

	*name = NULL;

	k_mutex_lock(&mutex, K_FOREVER);

	SYS_DLIST_FOR_EACH_NODE(&fs_mnt_list, node) {
		if (*index == cnt) {
			itr = CONTAINER_OF(node, struct fs_mount_t, node);
			break;
		}

		++cnt;
	}

	k_mutex_unlock(&mutex);

	if (itr != NULL) {
		rc = 0;
		*name = itr->mnt_point;
		++(*index);
	}

	return rc;

}