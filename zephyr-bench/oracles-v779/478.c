const void *usbh_desc_get_next_function(const void *const desc)
{
	const struct usb_desc_header *head = desc;
	const struct usb_association_descriptor *const ass_d = desc;
	const struct usb_if_descriptor *if_d;
	uint8_t skip_num = 0;

	/* Skip all interfaces the Association descriptor contains */
	if (usbh_desc_is_valid_association(head)) {
		skip_num = ass_d->bInterfaceCount;
	}

	/* Skip the interface if the head is interface */
	if (usbh_desc_is_valid_interface(head)) {
		skip_num = 1;
	}

	while (true) {
		/* If already on an Interface Association or Interface, this will skip it */
		head = usbh_desc_get_next(head);
		if (head == NULL) {
			break;
		}

		if_d = (const void *)head;

		/* Association descriptor: this is always a new function */
		if (usbh_desc_is_valid_association(head)) {
			return head;
		}

		/* Only count the first Alternate Setting of an Interface */
		if (usbh_desc_is_valid_interface(head) &&
		    if_d->bAlternateSetting == 0) {
			if (skip_num == 0) {
				return head;
			}

			skip_num--;
		}
	}

	return NULL;
}