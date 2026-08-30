ssize_t write(int fd, const void *buf, size_t sz)
{
	return zvfs_write(fd, buf, sz, NULL);
}