int coredump_query(enum coredump_query_id query_id, void *arg)
{
	int ret;

	if (backend_api->query == NULL) {
		ret = -ENOTSUP;
	} else {
		ret = backend_api->query(query_id, arg);
	}

	return ret;
}