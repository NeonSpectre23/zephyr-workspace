void zvfs_free_fd(int fd)
{
	/* Assumes fd was already bounds-checked. */
	(void)z_fd_unref(fd);
}