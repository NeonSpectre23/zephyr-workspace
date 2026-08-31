enum usbd_speed usbd_caps_speed(const struct usbd_context *const uds_ctx)
{
	struct udc_device_caps caps = udc_caps(uds_ctx->dev);

	/* For now, either high speed is supported or not. */
	if (caps.hs) {
		return USBD_SPEED_HS;
	}

	return USBD_SPEED_FS;
}