int usb_handle_os_desc_feature(struct usb_setup_packet *setup,
			       int32_t *len, uint8_t **data)
{
	LOG_DBG("bRequest 0x%x", setup->bRequest);

	if (!os_desc) {
		return -ENOTSUP;
	}

	if (setup->bRequest == os_desc->vendor_code) {
		switch (setup->wIndex) {
		case USB_OSDESC_EXTENDED_COMPAT_ID:
			LOG_DBG("Handle Compat ID");
			*data = os_desc->compat_id;
			*len = os_desc->compat_id_len;

			return 0;
		default:
			break;
		}
	}

	return -ENOTSUP;
}