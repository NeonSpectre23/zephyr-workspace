void *zvfs_get_fd_obj_and_vtable(int fd, const struct fd_op_vtable **vtable, struct k_mutex **lock)
{
	struct fd_entry *entry;

	if (_check_fd(fd) < 0) {
		return NULL;
	}

	entry = &fdtable[fd];
	*vtable = entry->vtable;

	if (lock != NULL) {
		*lock = &entry->lock;
	}

	return entry->obj;
}