uint32_t log_filter_get(struct log_backend const *const backend,
			uint32_t domain_id, int16_t source_id, bool runtime)
{
	int id = (backend == NULL) ? -1 : log_backend_id_get(backend);

	return filter_get(id, domain_id, source_id, runtime);
}