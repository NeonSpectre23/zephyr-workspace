int zvfs_reserve_fd(void)
{
	int fd;

	(void)k_mutex_lock(&fdtable_lock, K_FOREVER);

	fd = _find_fd_entry();
	if (fd >= 0) {
		/* Mark entry as used, zvfs_finalize_fd() will fill it in. */
		(void)z_fd_ref(fd);
		fdtable[fd].obj = NULL;
		fdtable[fd].vtable = NULL;
		fdtable[fd].offset = 0;
		k_mutex_init(&fdtable[fd].lock);
		k_condvar_init(&fdtable[fd].cond);
	}

	k_mutex_unlock(&fdtable_lock);

	return fd;
}