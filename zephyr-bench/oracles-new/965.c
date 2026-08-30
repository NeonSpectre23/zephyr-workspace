int usb_set_config(const uint8_t *device_descriptor)
{
	/* register descriptors */
	usb_register_descriptors(device_descriptor);

	/* register standard request handler */
	usb_register_request_handler(USB_REQTYPE_TYPE_STANDARD,
				     usb_handle_standard_request);

	/* register class request handlers for each interface*/
	usb_register_request_handler(USB_REQTYPE_TYPE_CLASS, class_handler);

	/* register vendor request handler */
	usb_register_request_handler(USB_REQTYPE_TYPE_VENDOR, vendor_handler);

	/* register class request handlers for each interface*/
	usb_register_custom_req_handler(custom_handler);

	return 0;
}