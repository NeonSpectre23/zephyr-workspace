int usbh_req_setup(struct usb_device *const udev,
		   const uint8_t bmRequestType,
		   const uint8_t bRequest,
		   const uint16_t wValue,
		   const uint16_t wIndex,
		   const uint16_t wLength,
		   struct net_buf *const buf)
{
	struct usb_setup_packet req = {
		.bmRequestType = bmRequestType,
		.bRequest = bRequest,
		.wValue = sys_cpu_to_le16(wValue),
		.wIndex = sys_cpu_to_le16(wIndex),
		.wLength = sys_cpu_to_le16(wLength),
	};
	struct uhc_transfer *xfer;
	uint8_t ep = usb_reqtype_is_to_device(&req) ? 0x00 : 0x80;
	int ret;

	xfer = usbh_xfer_alloc(udev, ep, ch9_req_cb, NULL);
	if (!xfer) {
		return -ENOMEM;
	}

	memcpy(xfer->setup_pkt, &req, sizeof(req));

	__ASSERT(IS_ENABLED(CONFIG_ZTEST) ||
		 (buf != NULL && wLength) || (buf == NULL && !wLength),
		 "Unresolved conditions for data stage");
	if (wLength) {
		ret = usbh_xfer_buf_add(udev, xfer, buf);
		if (ret) {
			goto buf_alloc_err;
		}
	}

	if (IS_ENABLED(CONFIG_ZTEST) && ctrl_req_no_status) {
		xfer->no_status = true;
	}

	ret = usbh_xfer_enqueue(udev, xfer);
	if (ret) {
		goto buf_alloc_err;
	}

	if (k_sem_take(&ch9_req_sync, K_MSEC(SETUP_REQ_TIMEOUT)) != 0) {
		ret = usbh_xfer_dequeue(udev, xfer);
		if (ret != 0) {
			LOG_ERR("Failed to cancel transfer");
			return ret;
		}

		LOG_ERR("Timeout");
		return -ETIMEDOUT;
	}

	ret = xfer->err;

buf_alloc_err:
	usbh_xfer_free(udev, xfer);

	return ret;
}