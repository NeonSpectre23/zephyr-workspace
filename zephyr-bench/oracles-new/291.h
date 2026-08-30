static inline void usbh_xfer_buf_free(const struct usb_device *udev,
				      struct net_buf *const buf)
{
	struct usbh_context *const ctx = udev->ctx;

	uhc_xfer_buf_free(ctx->dev, buf);
}