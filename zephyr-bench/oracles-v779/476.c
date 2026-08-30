const void *usbh_desc_get_next(const void *const desc)
{
	const struct usb_desc_header *const head = desc;
	const void *next;

	if (!usbh_desc_is_valid(desc, sizeof(const struct usb_desc_header), 0)) {
		return NULL;
	}

	next = (const uint8_t *)desc + head->bLength;

	if (!usbh_desc_is_valid(next, sizeof(const struct usb_desc_header), 0)) {
		return NULL;
	}

	return next;
}