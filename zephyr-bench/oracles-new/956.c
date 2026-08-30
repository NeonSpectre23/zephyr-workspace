int usb_disable(void)
{
	int ret;

	if (usb_dev.enabled != true) {
		/*Already disabled*/
		return 0;
	}

	ret = usb_dc_detach();
	if (ret < 0) {
		return ret;
	}

	usb_cancel_transfers();
	for (uint8_t i = 0; i <= 15; i++) {
		if (usb_dev.ep_bm & BIT(i)) {
			ret = disable_endpoint(i);
			if (ret < 0) {
				return ret;
			}
		}
		if (usb_dev.ep_bm & BIT(i + 16)) {
			ret = disable_endpoint(USB_EP_DIR_IN | i);
			if (ret < 0) {
				return ret;
			}
		}
	}

	/* Disable VBUS if needed */
	usb_vbus_set(false);

	usb_dev.enabled = false;

	return 0;
}