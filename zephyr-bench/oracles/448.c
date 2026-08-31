int usbh_shutdown(struct usbh_context *const uhs_ctx)
{
	int ret;

	usbh_host_lock(uhs_ctx);

	ret = uhc_shutdown(uhs_ctx->dev);
	if (ret) {
		LOG_ERR("Failed to shutdown USB device");
	}

	usbh_host_unlock(uhs_ctx);

	return ret;
}