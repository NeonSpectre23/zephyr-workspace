void log_backend_enable(struct log_backend const *const backend,
			void *ctx,
			uint32_t level)
{
	backend->cb->level = level;
	backend_filter_set(backend, level);
	log_backend_activate(backend, ctx);

	z_log_notify_backend_enabled();
}