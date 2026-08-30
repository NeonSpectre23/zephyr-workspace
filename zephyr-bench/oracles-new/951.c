int usb_dc_ep_read_wait(uint8_t ep, uint8_t *data, uint32_t max_data_len,
			uint32_t *read_bytes)
{
	struct usb_dwc2_reg *const base = usb_dw_cfg.base;
	uint8_t ep_idx = USB_EP_GET_IDX(ep);
	uint32_t i, j, data_len, bytes_to_copy;

	if (!usb_dw_ctrl.attached || !usb_dw_ep_is_valid(ep)) {
		LOG_ERR("Not attached / Invalid endpoint: EP 0x%x", ep);
		return -EINVAL;
	}

	/* Check if OUT ep */
	if (USB_EP_GET_DIR(ep) != USB_EP_DIR_OUT) {
		LOG_ERR("Wrong endpoint direction");
		return -EINVAL;
	}

	/* Allow to read 0 bytes */
	if (!data && max_data_len) {
		LOG_ERR("Wrong arguments");
		return -EINVAL;
	}

	/* Check if ep enabled */
	if (!usb_dw_ep_is_enabled(ep)) {
		LOG_ERR("Not enabled endpoint");
		return -EINVAL;
	}

	data_len = usb_dw_ctrl.out_ep_ctrl[ep_idx].data_len;

	if (!data && !max_data_len) {
		/* When both buffer and max data to read are zero return
		 * the available data in buffer
		 */
		if (read_bytes) {
			*read_bytes = data_len;
		}
		return 0;
	}


	if (data_len > max_data_len) {
		LOG_ERR("Not enough room to copy all the rcvd data!");
		bytes_to_copy = max_data_len;
	} else {
		bytes_to_copy = data_len;
	}

	LOG_DBG("Read EP%d, req %d, read %d bytes", ep, max_data_len,
		bytes_to_copy);

	/* Data in the FIFOs is always stored per 32-bit words */
	for (i = 0U; i < (bytes_to_copy & ~0x3); i += 4U) {
		*(uint32_t *)(data + i) = USB_DW_EP_FIFO(base, ep_idx);
	}
	if (bytes_to_copy & 0x3) {
		/* Not multiple of 4 */
		uint32_t last_dw = USB_DW_EP_FIFO(base, ep_idx);

		for (j = 0U; j < (bytes_to_copy & 0x3); j++) {
			*(data + i + j) =
				(sys_cpu_to_le32(last_dw) >> (j * 8U)) & 0xFF;
			}
	}

	usb_dw_ctrl.out_ep_ctrl[ep_idx].data_len -= bytes_to_copy;

	if (read_bytes) {
		*read_bytes = bytes_to_copy;
	}

	return 0;

}