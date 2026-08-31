int usb_deconfig(void)
{
	/* unregister descriptors */
	usb_register_descriptors(NULL);

	/* unregister standard request handler */
	usb_register_request_handler(USB_REQTYPE_TYPE_STANDARD, NULL);

	/* unregister class request handlers for each interface*/
	usb_register_request_handler(USB_REQTYPE_TYPE_CLASS, NULL);

	/* unregister class request handlers for each interface*/
	usb_register_custom_req_handler(NULL);

	/* unregister status callback */
	usb_register_status_callback(NULL);

	/* unregister user status callback */
	usb_dev.user_status_callback = NULL;

	/* Reset USB controller */
	usb_dc_reset();

	return 0;
}