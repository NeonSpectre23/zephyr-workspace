int usbh_enable(struct usbh_context *uhs_ctx)
{
	int ret;

	usbh_host_lock(uhs_ctx);

	if (!uhc_is_initialized(uhs_ctx->dev)) {
		LOG_WRN("USB host controller is not initialized");
		ret = -EPERM;
		goto enable_exit;
	}

	if (uhc_is_enabled(uhs_ctx->dev)) {
		LOG_WRN("USB host controller is already enabled");
		ret = -EALREADY;
		goto enable_exit;
	}

	ret = uhc_enable(uhs_ctx->dev);
	if (ret != 0) {
		LOG_ERR("Failed to enable controller");
		goto enable_exit;
	}

enable_exit:
	usbh_host_unlock(uhs_ctx);
	return ret;
}