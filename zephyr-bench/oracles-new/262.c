const struct log_backend *log_format_set_all_active_backends(size_t log_type)
{
	const struct log_backend *failed_backend = NULL;

	STRUCT_SECTION_FOREACH(log_backend, backend) {
		if (log_backend_is_active(backend)) {
			int retCode = log_backend_format_set(backend, log_type);

			if (retCode != 0) {
				failed_backend = backend;
			}
		}
	}
	return failed_backend;
}