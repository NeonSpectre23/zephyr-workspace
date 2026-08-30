int usb_dc_ep_halt(const uint8_t ep)
{
	struct usb_dwc2_reg *const base = usb_dw_cfg.base;
	uint8_t ep_idx = USB_EP_GET_IDX(ep);
	volatile uint32_t *p_depctl;

	if (!usb_dw_ctrl.attached || !usb_dw_ep_is_valid(ep)) {
		LOG_ERR("Not attached / Invalid endpoint: EP 0x%x", ep);
		return -EINVAL;
	}

	if (!ep_idx) {
		/* Cannot disable EP0, just set stall */
		usb_dc_ep_set_stall(ep);
	} else {
		if (USB_EP_DIR_IS_OUT(ep)) {
			p_depctl = &base->out_ep[ep_idx].doepctl;
		} else {
			p_depctl = &base->in_ep[ep_idx].diepctl;
		}

		/* Set STALL and disable endpoint if enabled */
		if (*p_depctl & USB_DWC2_DEPCTL_EPENA) {
			*p_depctl |= USB_DWC2_DEPCTL_EPDIS | USB_DWC2_DEPCTL_STALL;
		} else {
			*p_depctl |= USB_DWC2_DEPCTL_STALL;
		}
	}

	return 0;
}