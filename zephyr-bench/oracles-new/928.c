void tracing_format_string(const char *str, ...)
{
	va_list args;
	bool put_success, before_put_is_empty;

	if (!is_tracing_enabled() || is_tracing_thread()) {
		return;
	}

	va_start(args, str);

	TRACING_LOCK();
	before_put_is_empty = tracing_buffer_is_empty();
	put_success = tracing_format_string_put(str, args);
	TRACING_UNLOCK();

	va_end(args);

	if (put_success) {
		tracing_trigger_output(before_put_is_empty);
	} else {
		tracing_packet_drop_handle();
	}
}