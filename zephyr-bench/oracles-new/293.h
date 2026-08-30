static inline void z_log_msg_runtime_create(uint8_t domain_id,
					     const void *source,
					     uint8_t level, const void *data,
					     size_t dlen, uint32_t package_flags,
					     const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	z_log_msg_runtime_vcreate(domain_id, source, level,
				   data, dlen, package_flags, fmt, ap);
	va_end(ap);
}