void log_backend_disable(struct log_backend const *const backend)
{
	if (log_backend_is_active(backend)) {
		backend_filter_set(backend, LOG_LEVEL_NONE);
	}

	log_backend_deactivate(backend);
}