static inline uint32_t log_const_source_id(
				const struct log_source_const_data *data)
{
	return ((const uint8_t *)data - (uint8_t *)TYPE_SECTION_START(log_const))/
			sizeof(struct log_source_const_data);
}