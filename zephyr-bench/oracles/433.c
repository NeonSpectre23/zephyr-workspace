int usbd_shutdown(struct usbd_context *const uds_ctx)
{
	int ret;

	usbd_device_lock(uds_ctx);

	ret = usbd_device_shutdown_core(uds_ctx);
	if (ret) {
		LOG_ERR("Failed to shutdown USB device");
	}

	ret = udc_purge_queues(uds_ctx->dev);
	if (ret) {
		LOG_ERR("Failed to purge endpoint queues");
	}

	usbd_free_preallocated(uds_ctx);

	uds_ctx->status.initialized = false;
	usbd_device_unlock(uds_ctx);

	return 0;
}