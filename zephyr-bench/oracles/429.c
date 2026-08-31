int usbd_enable(struct usbd_context *const uds_ctx)
{
	bool ep0_empty;
	int ret;

	k_sched_lock();
	usbd_device_lock(uds_ctx);

	if (!usbd_is_initialized(uds_ctx)) {
		LOG_WRN("USB device support is not initialized");
		ret = -EPERM;
		goto enable_exit;
	}

	if (usbd_is_enabled(uds_ctx)) {
		LOG_WRN("USB device support is already enabled");
		ret = -EALREADY;
		goto enable_exit;
	}

	/* UDC drivers can keep enqueued control buffers across disable/enable
	 * cycle. Enqueue SETUP buffer only if there are no queued buffers.
	 * This check has to be done before udc_enable(), because only before
	 * enable the driver won't complete any queued buffer.
	 */
	ep0_empty = udc_ep_queue_is_empty(uds_ctx->dev, USB_CONTROL_EP_OUT);

	ret = udc_enable(uds_ctx->dev);
	if (ret != 0) {
		LOG_ERR("Failed to enable controller");
		goto enable_exit;
	}

	ret = usbd_preallocate(uds_ctx);
	if (ret != 0) {
		LOG_ERR("Buffer preallocation failed");
		udc_disable(uds_ctx->dev);
		goto enable_exit;
	}

	ret = usbd_init_control_pipe(uds_ctx, ep0_empty);
	if (ret != 0) {
		udc_disable(uds_ctx->dev);
		goto enable_exit;
	}

	uds_ctx->status.enabled = true;

enable_exit:
	usbd_device_unlock(uds_ctx);
	k_sched_unlock();

	return ret;
}