uint64_t log_output_timestamp_to_us(log_timestamp_t timestamp)
{
	timestamp /= timestamp_div;

	return ((uint64_t) timestamp * 1000000U) / freq;
}