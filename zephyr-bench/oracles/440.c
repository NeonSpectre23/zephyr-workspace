struct usb_device *usbh_device_get_any(struct usbh_context *const uhs_ctx)
{
	sys_dnode_t *const node = sys_dlist_peek_head(&uhs_ctx->udevs);
	struct usb_device *udev;

	udev = SYS_DLIST_CONTAINER(node, udev, node);

	return udev;
}