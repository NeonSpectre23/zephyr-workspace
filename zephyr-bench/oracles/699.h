static inline struct net_buf *usbh_xfer_buf_alloc(struct usb_device *udev,
						  const size_t size)
{
	struct usbh_context *const ctx = udev->ctx;

	return uhc_xfer_buf_alloc(ctx->dev, size);
}