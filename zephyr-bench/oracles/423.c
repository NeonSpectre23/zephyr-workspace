int usb_handle_os_desc(struct usb_setup_packet *setup,
		       int32_t *len, uint8_t **data)
{
	if (!os_desc) {
		return -ENOTSUP;
	}

	if (USB_GET_DESCRIPTOR_TYPE(setup->wValue) == USB_DESC_STRING &&
	    USB_GET_DESCRIPTOR_INDEX(setup->wValue) == USB_OSDESC_STRING_DESC_INDEX) {
		LOG_DBG("MS OS Descriptor string read");
		*data = os_desc->string;
		*len = os_desc->string_len;

		return 0;
	}

	return -ENOTSUP;
}