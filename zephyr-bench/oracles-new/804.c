int stream_flash_buffered_write(struct stream_flash_ctx *ctx, const uint8_t *data,
				size_t len, bool flush)
{
	int processed = 0;
	int rc = 0;
	int buf_empty_bytes;

	if (!ctx) {
		return -EFAULT;
	}

	if (ctx->bytes_written + ctx->buf_bytes + len > ctx->available) {
		return -ENOMEM;
	}

	while ((len - processed) >=
	       (buf_empty_bytes = ctx->buf_len - ctx->buf_bytes)) {
		memcpy(ctx->buf + ctx->buf_bytes, data + processed,
		       buf_empty_bytes);

		ctx->buf_bytes = ctx->buf_len;
		rc = flash_sync(ctx);

		if (rc != 0) {
			return rc;
		}

		processed += buf_empty_bytes;
	}

	/* place rest of the data into ctx->buf */
	if (processed < len) {
		memcpy(ctx->buf + ctx->buf_bytes,
		       data + processed, len - processed);
		ctx->buf_bytes += len - processed;
	}

	if (flush && ctx->buf_bytes > 0) {
		rc = flash_sync(ctx);
	}

	return rc;
}