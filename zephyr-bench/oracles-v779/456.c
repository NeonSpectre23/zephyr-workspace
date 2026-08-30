int usb_handle_bos(struct usb_setup_packet *setup,
		   int32_t *len, uint8_t **data)
{
	if (USB_GET_DESCRIPTOR_TYPE(setup->wValue) == USB_DESC_BOS) {
		LOG_DBG("Read BOS descriptor");
		*data = (uint8_t *)usb_bos_get_header();
		*len = usb_bos_get_length();

		return 0;
	}

	return -ENOTSUP;
}