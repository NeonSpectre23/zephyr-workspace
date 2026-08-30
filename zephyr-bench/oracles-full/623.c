int zvfs_alloc_fd(void *obj, const struct fd_op_vtable *vtable)
{
	int fd;

	fd = zvfs_reserve_fd();
	if (fd >= 0) {
		zvfs_finalize_fd(fd, obj, vtable);
	}

	return fd;
}