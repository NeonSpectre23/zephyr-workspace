int usb_enable(usb_dc_status_callback status_cb)
{
	int ret;
	struct usb_dc_ep_cfg_data ep0_cfg;
	struct usb_device_descriptor *dev_desc;

	/* Prevent from calling usb_enable form different context.
	 * This should only be called once.
	 */
	LOG_DBG("lock usb_enable_lock mutex");
	k_mutex_lock(&usb_enable_lock, K_FOREVER);

	if (usb_dev.enabled == true) {
		LOG_WRN("USB device support already enabled");
		ret = -EALREADY;
		goto out;
	}

	/*
	 * If usb_dev.descriptors is equal to NULL (usb_dev has static
	 * specifier), then usb_get_device_descriptor() and usb_set_config()
	 * are likely not called yet. If so, set the configuration here.
	 */
	if (usb_dev.descriptors == NULL) {
		usb_set_config(usb_get_device_descriptor());
		if (usb_dev.descriptors == NULL) {
			LOG_ERR("Failed to configure USB device stack");
			ret =  -1;
			goto out;
		}
	}

	/* Enable VBUS if needed */
	ret = usb_vbus_set(true);
	if (ret < 0) {
		goto out;
	}

	dev_desc = (void *)usb_dev.descriptors;
	usb_dev.user_status_callback = status_cb;
	usb_register_status_callback(forward_status_cb);
	usb_dc_set_status_callback(forward_status_cb);

	ret = usb_dc_attach();
	if (ret < 0) {
		goto out;
	}

	ret = usb_transfer_init();
	if (ret < 0) {
		goto out;
	}

	if (dev_desc->bDescriptorType != USB_DESC_DEVICE ||
	    dev_desc->bMaxPacketSize0 == 0) {
		LOG_ERR("Erroneous device descriptor or bMaxPacketSize0");
		ret = -EINVAL;
		goto out;
	}

	/* Configure control EP */
	usb_dev.mps0 = dev_desc->bMaxPacketSize0;
	ep0_cfg.ep_mps = usb_dev.mps0;
	ep0_cfg.ep_type = USB_DC_EP_CONTROL;

	ep0_cfg.ep_addr = USB_CONTROL_EP_OUT;
	ret = usb_dc_ep_configure(&ep0_cfg);
	if (ret < 0) {
		goto out;
	}

	ep0_cfg.ep_addr = USB_CONTROL_EP_IN;
	ret = usb_dc_ep_configure(&ep0_cfg);
	if (ret < 0) {
		goto out;
	}

	/* Register endpoint 0 handlers*/
	ret = usb_dc_ep_set_callback(USB_CONTROL_EP_OUT,
				     usb_handle_control_transfer);
	if (ret < 0) {
		goto out;
	}

	ret = usb_dc_ep_set_callback(USB_CONTROL_EP_IN,
				     usb_handle_control_transfer);
	if (ret < 0) {
		goto out;
	}

	/* Register endpoint handlers*/
	ret = composite_setup_ep_cb();
	if (ret < 0) {
		goto out;
	}

	/* Enable control EP */
	ret = usb_dc_ep_enable(USB_CONTROL_EP_OUT);
	if (ret < 0) {
		goto out;
	}
	usb_dev.ep_bm |= get_ep_bm_from_addr(USB_CONTROL_EP_OUT);

	ret = usb_dc_ep_enable(USB_CONTROL_EP_IN);
	if (ret < 0) {
		goto out;
	}
	usb_dev.ep_bm |= get_ep_bm_from_addr(USB_CONTROL_EP_IN);

	usb_dev.enabled = true;
	ret = 0;
out:
	LOG_DBG("unlock usb_enable_lock mutex");
	k_mutex_unlock(&usb_enable_lock);
	return ret;
}