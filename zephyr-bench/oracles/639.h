static inline const struct shell *shell_backend_get(uint32_t idx)
{
	const struct shell *backend;

	STRUCT_SECTION_GET(shell, idx, &backend);

	return backend;
}