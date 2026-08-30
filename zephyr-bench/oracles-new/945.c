int usb_dc_ep_flush(const uint8_t ep)
{
	struct usb_dwc2_reg *const base = usb_dw_cfg.base;
	uint8_t ep_idx = USB_EP_GET_IDX(ep);
	uint32_t cnt;

	if (!usb_dw_ctrl.attached || !usb_dw_ep_is_valid(ep)) {
		LOG_ERR("Not attached / Invalid endpoint: EP 0x%x", ep);
		return -EINVAL;
	}

	if (USB_EP_DIR_IS_OUT(ep)) {
		/* RX FIFO is global and cannot be flushed per EP */
		return -EINVAL;
	}

	/* Each endpoint has dedicated Tx FIFO */
	base->grstctl |= ep_idx << USB_DWC2_GRSTCTL_TXFNUM_POS;
	base->grstctl |= USB_DWC2_GRSTCTL_TXFFLSH;

	cnt = 0U;

	do {
		if (++cnt > USB_DW_CORE_RST_TIMEOUT_US) {
			LOG_ERR("USB TX FIFO flush HANG!");
			return -EIO;
		}
		usb_dw_udelay(1);
	} while (base->grstctl & USB_DWC2_GRSTCTL_TXFFLSH);

	return 0;
}