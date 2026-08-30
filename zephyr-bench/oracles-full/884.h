static inline void zvfs_finalize_fd(int fd, void *obj, const struct fd_op_vtable *vtable)
{
	zvfs_finalize_typed_fd(fd, obj, vtable, ZVFS_MODE_UNSPEC);
}