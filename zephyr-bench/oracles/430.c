int usbd_init(struct usbd_context *const uds_ctx)
{
	int ret;

	/*
	 * Lock the scheduler to ensure that the context is not preempted
	 * before it is fully initialized.
	 */
	k_sched_lock();
	usbd_device_lock(uds_ctx);

	if (uds_ctx->dev == NULL) {
		ret = -ENODEV;
		goto init_exit;
	}

	if (usbd_is_initialized(uds_ctx)) {
		LOG_WRN("USB device support is already initialized");
		ret = -EALREADY;
		goto init_exit;
	}

	if (!device_is_ready(uds_ctx->dev)) {
		LOG_ERR("USB device controller is not ready");
		ret = -ENODEV;
		goto init_exit;
	}

	ret = usbd_device_init_core(uds_ctx);
	if (ret) {
		goto init_exit;
	}

	memset(&uds_ctx->ch9_data, 0, sizeof(struct usbd_ch9_data));
	uds_ctx->status.initialized = true;

init_exit:
	usbd_device_unlock(uds_ctx);
	k_sched_unlock();

	return ret;
}