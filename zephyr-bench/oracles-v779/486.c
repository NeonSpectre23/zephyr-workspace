int usbh_req_set_alt(struct usb_device *const udev,
		     const uint8_t iface, const uint8_t alt)
{
	const uint8_t bmRequestType = USB_REQTYPE_DIR_TO_DEVICE << 7 |
				      USB_REQTYPE_RECIPIENT_INTERFACE;
	const uint8_t bRequest = USB_SREQ_SET_INTERFACE;
	const uint16_t wValue = alt;
	const uint16_t wIndex = iface;

	return usbh_req_setup(udev,
			      bmRequestType, bRequest, wValue, wIndex, 0,
			      NULL);
}