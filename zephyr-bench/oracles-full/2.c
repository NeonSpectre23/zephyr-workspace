static const struct log_backend *backend_find(char const *name)
{
	size_t slen = strlen(name);

	STRUCT_SECTION_FOREACH(log_backend, backend) {
		if (strncmp(name, backend->name, slen) == 0) {
			return backend;
		}
	}

	return NULL;
}