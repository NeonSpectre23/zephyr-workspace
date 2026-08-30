int coredump_cmd(enum coredump_cmd_id cmd_id, void *arg)
{
	int ret;

	if (backend_api->cmd == NULL) {
		ret = -ENOTSUP;
	} else {
		ret = backend_api->cmd(cmd_id, arg);
	}

	return ret;
}