int usb_dc_ep_set_stall(const uint8_t ep)
{
	struct usb_dwc2_reg *const base = usb_dw_cfg.base;
	uint8_t ep_idx = USB_EP_GET_IDX(ep);

	if (!usb_dw_ctrl.attached || !usb_dw_ep_is_valid(ep)) {
		LOG_ERR("Not attached / Invalid endpoint: EP 0x%x", ep);
		return -EINVAL;
	}

	if (USB_EP_DIR_IS_OUT(ep)) {
		base->out_ep[ep_idx].doepctl |= USB_DWC2_DEPCTL_STALL;
	} else {
		base->in_ep[ep_idx].diepctl |= USB_DWC2_DEPCTL_STALL;
	}

	return 0;
}