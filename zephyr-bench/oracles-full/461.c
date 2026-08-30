int stream_flash_progress_clear(const struct stream_flash_ctx *ctx,
				const char *settings_key)
{
	if (!ctx || !settings_key) {
		return -EFAULT;
	}

	int rc = stream_flash_settings_init();

	if (rc == 0) {
		rc = settings_delete(settings_key);
	}

	if (rc != 0) {
		LOG_ERR("Error %d while deleting progress for \"%s\"",
			rc, settings_key);
	}

	return rc;
}