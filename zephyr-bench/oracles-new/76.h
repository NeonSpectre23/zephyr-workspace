static inline void log_backend_deactivate(
				const struct log_backend *const backend)
{
	__ASSERT_NO_MSG(backend != NULL);
	backend->cb->active = false;
}