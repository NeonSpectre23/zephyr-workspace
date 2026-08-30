ssize_t read(int fd, void *buf, size_t sz)
{
	return zvfs_read(fd, buf, sz, NULL);
}