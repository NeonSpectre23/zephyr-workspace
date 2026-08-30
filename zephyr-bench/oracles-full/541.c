int usbd_disable(struct usbd_context *const uds_ctx)
{
	int ret;

	if (!usbd_is_enabled(uds_ctx)) {
		LOG_WRN("USB device support is already disabled");
		return -EALREADY;
	}

	usbd_device_lock(uds_ctx);

	ret = usbd_config_set(uds_ctx, 0);
	if (ret) {
		LOG_ERR("Failed to reset configuration");
	}

	ret = udc_disable(uds_ctx->dev);
	if (ret) {
		LOG_ERR("Failed to disable USB device");
	}

	uds_ctx->status.enabled = false;

	usbd_device_unlock(uds_ctx);

	return ret;
}