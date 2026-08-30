static inline int log_backend_format_set(const struct log_backend *backend, uint32_t log_type)
{
	extern size_t log_format_table_size(void);

	if ((size_t)log_type >= log_format_table_size()) {
		return -EINVAL;
	}

	if (log_format_func_t_get(log_type) == NULL) {
		return -EINVAL;
	}

	if (backend == NULL) {
		return -EINVAL;
	}

	if (backend->api->format_set == NULL) {
		return -ENOTSUP;
	}

	return backend->api->format_set(backend, log_type);
}