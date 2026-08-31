int usbd_register_class(struct usbd_context *const uds_ctx,
			const char *name,
			const enum usbd_speed speed, const uint8_t cfg)
{
	struct usbd_class_node *c_nd;
	struct usbd_class_data *c_data;
	int ret;

	c_nd = usbd_class_node_get(name, speed);
	if (c_nd == NULL) {
		return -ENODEV;
	}

	usbd_device_lock(uds_ctx);

	if (usbd_is_initialized(uds_ctx)) {
		LOG_ERR("USB device support is initialized");
		ret = -EBUSY;
		goto register_class_error;
	}

	c_data = c_nd->c_data;

	/* TODO: does it still need to be atomic ? */
	if (atomic_test_bit(&c_nd->state, USBD_CCTX_REGISTERED)) {
		LOG_WRN("Class instance already registered");
		ret = -EBUSY;
		goto register_class_error;
	}

	if ((c_data->uds_ctx != NULL) && (c_data->uds_ctx != uds_ctx)) {
		LOG_ERR("Class registered to other context at different speed");
		ret = -EBUSY;
		goto register_class_error;
	}

	ret = usbd_class_append(uds_ctx, c_nd, speed, cfg);
	if (ret == 0) {
		/* Initialize pointer back to the device struct */
		atomic_set_bit(&c_nd->state, USBD_CCTX_REGISTERED);
		c_data->uds_ctx = uds_ctx;
	}

register_class_error:
	usbd_device_unlock(uds_ctx);
	return ret;
}