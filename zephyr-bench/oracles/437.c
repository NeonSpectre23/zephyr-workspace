const void *usbh_desc_get_iface(const struct usb_device *const udev, const uint8_t iface)
{
	const struct usb_cfg_descriptor *const c_desc = udev->cfg_desc;

	for (unsigned int i = 0; i < c_desc->bNumInterfaces; i++) {
		const struct usb_host_interface *const host_iface = &udev->ifaces[i];
		const struct usb_if_descriptor *const if_d = (void *)host_iface->dhp;

		if (if_d->bInterfaceNumber == iface) {
			return host_iface->dhp;
		}
	}

	return NULL;
}