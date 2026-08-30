static inline void log_backend_activate(const struct log_backend *const backend,
					void *ctx)
{
	__ASSERT_NO_MSG(backend != NULL);
	backend->cb->ctx = ctx;
	backend->cb->active = true;
}