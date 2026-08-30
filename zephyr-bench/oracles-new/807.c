int stream_flash_init(struct stream_flash_ctx *ctx, const struct device *fdev,
		      uint8_t *buf, size_t buf_len, size_t offset, size_t size,
		      stream_flash_callback_t cb)
{
	const struct flash_parameters *params;

	if (!ctx || !fdev || !buf) {
		return -EFAULT;
	}

	params = flash_get_parameters(fdev);

	if (buf_len % params->write_block_size) {
		LOG_ERR("Buffer size is not aligned to minimal write-block-size");
		return -EFAULT;
	}

	if (offset % params->write_block_size) {
		LOG_ERR("Incorrect parameter");
		return -EFAULT;
	}

	if (size == 0 || size % params->write_block_size) {
		LOG_ERR("Size is incorrect");
		return -EFAULT;
	}

	if ((offset + size) < offset) {
		LOG_ERR("Requested range overflows SIZE_MAX");
		return -EFAULT;
	}

	ctx->fdev = fdev;
	ctx->buf = buf;
	ctx->buf_len = buf_len;
	ctx->bytes_written = 0;
	ctx->buf_bytes = 0U;
	ctx->offset = offset;
	ctx->available = size;
	ctx->write_block_size = params->write_block_size;

#if !defined(CONFIG_STREAM_FLASH_POST_WRITE_CALLBACK)
	ARG_UNUSED(cb);
#else
	ctx->callback = cb;
#endif


#ifdef CONFIG_STREAM_FLASH_ERASE
	ctx->erased_up_to = 0;
#endif
	ctx->erase_value = params->erase_value;

	/* Inspection is deliberately done once context has been filled in */
	if (IS_ENABLED(CONFIG_STREAM_FLASH_INSPECT)) {
		int ret  = inspect_device(ctx);

		if (ret != 0) {
			/* No log here, the inspect_device already does logging */
			return ret;
		}
	}


	return 0;
}