static int check_buffer(const struct adc_sequence *sequence,
			     uint8_t active_channels)
{
	size_t needed_buffer_size;

	needed_buffer_size = active_channels * sizeof(adc_data_size_t);

	if (sequence->options) {
		needed_buffer_size *= (1 + sequence->options->extra_samplings);
	}

	if (sequence->buffer_size < needed_buffer_size) {
		LOG_ERR("Provided buffer is too small (%u/%u)",
				sequence->buffer_size, needed_buffer_size);
		return -ENOMEM;
	}

#if defined(CONFIG_ADC_STM32_DMA)
	/* Buffer is forced to be in non-cacheable SRAM region to avoid cache maintenance */
	if (!stm32_buf_in_nocache((uintptr_t)sequence->buffer, needed_buffer_size)) {
		LOG_ERR("Supplied buffer is not in a non-cacheable region.");
		return -EINVAL;
	}
#endif /* CONFIG_ADC_STM32_DMA */

	return 0;
}