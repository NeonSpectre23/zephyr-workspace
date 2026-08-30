void tracing_format_data(tracing_data_t *tracing_data_array, uint32_t count)
{
	bool put_success, before_put_is_empty;

	if (!is_tracing_enabled() || is_tracing_thread()) {
		return;
	}

	TRACING_LOCK();
	before_put_is_empty = tracing_buffer_is_empty();
	put_success = tracing_format_data_put(tracing_data_array, count);
	TRACING_UNLOCK();

	if (put_success) {
		tracing_trigger_output(before_put_is_empty);
	} else {
		tracing_packet_drop_handle();
	}
}