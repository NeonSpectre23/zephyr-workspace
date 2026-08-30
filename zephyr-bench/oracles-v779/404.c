int stream_flash_progress_save(const struct stream_flash_ctx *ctx,
			       const char *settings_key)
{
	if (!ctx || !settings_key) {
		return -EFAULT;
	}

	int rc = stream_flash_settings_init();

	if (rc == 0) {
		rc = settings_save_one(settings_key, &ctx->bytes_written,
				       sizeof(ctx->bytes_written));
	}

	if (rc != 0) {
		LOG_ERR("Error %d while storing progress for \"%s\"",
			rc, settings_key);
	}

	return rc;
}