static inline const struct log_backend *log_backend_get(uint32_t idx)
{
	const struct log_backend *backend;

	STRUCT_SECTION_GET(log_backend, idx, &backend);

	return backend;
}