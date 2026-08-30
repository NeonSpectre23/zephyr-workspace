uint32_t tracing_cmd_buffer_alloc(uint8_t **data)
{
	*data = &tracing_cmd_buffer[0];

	return sizeof(tracing_cmd_buffer);
}