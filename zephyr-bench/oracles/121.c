void log_output_process(const struct log_output *output,
			log_timestamp_t timestamp,
			const char *domain,
			const char *source,
			k_tid_t tid,
			uint8_t core_id,
			uint8_t level,
			const uint8_t *package,
			const uint8_t *data,
			size_t data_len,
			uint32_t flags)
{
	bool raw_string = (level == LOG_LEVEL_INTERNAL_RAW_STRING);
	uint32_t prefix_offset;
	cbprintf_cb cb;

	if (!raw_string) {
		prefix_offset = prefix_print(output, flags, 0, timestamp,
					     domain, source, tid, core_id, level);
		cb = out_func;
	} else {
		prefix_offset = 0;
		/* source set to 1 indicates raw string and contrary to printk
		 * case it should not append anything to the output (printk is
		 * appending <CR> to the new line character).
		 */
		cb = ((uintptr_t)source == 1) ? out_func : cr_out_func;
	}

	if (package) {
		int err = cbpprintf(cb, (void *)output, (void *)package);

		(void)err;
		__ASSERT_NO_MSG(err >= 0);
	}

	if (data_len) {
		log_msg_hexdump(output, (uint8_t *)data, data_len, prefix_offset, flags);
	}

	if (!raw_string) {
		postfix_print(output, flags, level);
	}

	log_output_flush(output);
}