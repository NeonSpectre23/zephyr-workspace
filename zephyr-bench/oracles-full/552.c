const void *usbh_desc_get_next_alt_setting(const void *const desc)
{
	const struct usb_desc_header *head = desc;

	/* Skip the current interface descriptor */
	head = usbh_desc_get_next(desc);

	/* Seek to the next alternate setting for this interface */
	for (; head != NULL; head = usbh_desc_get_next(head)) {
		struct usb_if_descriptor *if_d = (void *)head;

		if (head->bDescriptorType != USB_DESC_INTERFACE) {
			continue;
		}

		/* Non-zero Alternate Setting */
		if (usbh_desc_is_valid_interface(desc) && if_d->bAlternateSetting != 0) {
			return head;
		}

		/* Do not continue to the next interface */
		return NULL;
	}

	return NULL;
}