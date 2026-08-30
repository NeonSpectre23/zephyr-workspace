void tracing_format_raw_data(uint8_t *data, uint32_t length)
{
	bool put_success, before_put_is_empty;

	if (!is_tracing_enabled() || is_tracing_thread()) {
		return;
	}

	TRACING_LOCK();
	before_put_is_empty = tracing_buffer_is_empty();
	put_success = tracing_format_raw_data_put(data, length);
	TRACING_UNLOCK();

	if (put_success) {
		tracing_trigger_output(before_put_is_empty);
	} else {
		tracing_packet_drop_handle();
	}
}