int usbh_init(struct usbh_context *uhs_ctx)
{
	int ret;

	usbh_host_lock(uhs_ctx);

	if (!device_is_ready(uhs_ctx->dev)) {
		LOG_ERR("USB host controller is not ready");
		ret = -ENODEV;
		goto init_exit;
	}

	if (uhc_is_initialized(uhs_ctx->dev)) {
		LOG_WRN("USB host controller is already initialized");
		ret = -EALREADY;
		goto init_exit;
	}

	ret = usbh_init_device_intl(uhs_ctx);

init_exit:
	usbh_host_unlock(uhs_ctx);
	return ret;
}