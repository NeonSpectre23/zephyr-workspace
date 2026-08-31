const void *usbh_desc_get_endpoint(const struct usb_device *const udev, const uint8_t ep)
{
	uint8_t idx = USB_EP_GET_IDX(ep) & 0xf;

	return USB_EP_DIR_IS_IN(ep) ? udev->ep_in[idx].desc : udev->ep_out[idx].desc;
}