static inline bool log_backend_is_active(
				const struct log_backend *const backend)
{
	__ASSERT_NO_MSG(backend != NULL);
	return backend->cb->active;
}