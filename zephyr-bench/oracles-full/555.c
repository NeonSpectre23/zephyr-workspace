int usbh_disable(struct usbh_context *uhs_ctx)
{
	int ret;

	if (!uhc_is_enabled(uhs_ctx->dev)) {
		LOG_WRN("USB host controller is already disabled");
		return 0;
	}

	usbh_host_lock(uhs_ctx);

	ret = uhc_disable(uhs_ctx->dev);
	if (ret) {
		LOG_ERR("Failed to disable USB controller");
	}

	usbh_host_unlock(uhs_ctx);

	return 0;
}