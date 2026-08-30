int stream_flash_progress_load(struct stream_flash_ctx *ctx,
			       const char *settings_key)
{
	if (!ctx || !settings_key) {
		return -EFAULT;
	}

	int rc = stream_flash_settings_init();

	if (rc == 0) {
		rc = settings_load_subtree_direct(settings_key, settings_direct_loader,
						  (void *)ctx);
	}

	if (rc != 0) {
		LOG_ERR("Error %d while loading progress for \"%s\"",
			rc, settings_key);
	}

	return rc;
}