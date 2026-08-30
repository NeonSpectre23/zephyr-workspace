int usbh_req_desc(struct usb_device *const udev,
		  const uint8_t type, const uint8_t index,
		  const uint16_t id,
		  const uint16_t len,
		  struct net_buf *const buf)
{
	const uint8_t bmRequestType = USB_REQTYPE_DIR_TO_HOST << 7;
	const uint8_t bRequest = USB_SREQ_GET_DESCRIPTOR;
	const uint16_t wValue = (type << 8) | index;

	return usbh_req_setup(udev,
			      bmRequestType, bRequest, wValue, id, len,
			      buf);
}