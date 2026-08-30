const struct log_backend *log_backend_get_by_name(const char *backend_name)
{
	STRUCT_SECTION_FOREACH(log_backend, backend) {
		if (strcmp(backend_name, backend->name) == 0) {
			return backend;
		}
	}

	return NULL;
}