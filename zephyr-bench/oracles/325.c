int remove(const char *path)
{
	if (!IS_ENABLED(CONFIG_FILE_SYSTEM)) {
		errno = ENOTSUP;
		return -1;
	}

	int ret = fs_unlink(path);

	if (ret < 0) {
		errno = -ret;
		return -1;
	}

	return 0;
}