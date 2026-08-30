void log_output_timestamp_freq_set(uint32_t frequency)
{
	timestamp_div = 1U;
	/* There is no point to have frequency higher than 1MHz (ns are not
	 * printed) and too high frequency leads to overflows in calculations.
	 */
	while (frequency > 1000000) {
		frequency /= 2U;
		timestamp_div *= 2U;
	}

	freq = frequency;
}