void *zvfs_get_fd_obj(int fd, const struct fd_op_vtable *vtable, int err)
{
	struct fd_entry *entry;

	if (_check_fd(fd) < 0) {
		return NULL;
	}

	entry = &fdtable[fd];

	if ((vtable != NULL) && (entry->vtable != vtable)) {
		errno = err;
		return NULL;
	}

	return entry->obj;
}