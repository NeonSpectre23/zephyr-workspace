const struct shell *shell_backend_get_by_name(const char *backend_name)
{
	STRUCT_SECTION_FOREACH(shell, backend) {
		if (strcmp(backend_name, backend->name) == 0) {
			return backend;
		}
	}

	return NULL;
}

