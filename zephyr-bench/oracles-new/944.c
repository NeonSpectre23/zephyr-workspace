int usb_dc_ep_enable(const uint8_t ep)
{
	struct usb_dwc2_reg *const base = usb_dw_cfg.base;
	uint8_t ep_idx = USB_EP_GET_IDX(ep);

	if (!usb_dw_ctrl.attached || !usb_dw_ep_is_valid(ep)) {
		LOG_ERR("Not attached / Invalid endpoint: EP 0x%x", ep);
		return -EINVAL;
	}

	/* enable EP interrupts */
	if (USB_EP_DIR_IS_OUT(ep)) {
		base->daintmsk |= USB_DWC2_DAINT_OUTEPINT(ep_idx);
	} else {
		base->daintmsk |= USB_DWC2_DAINT_INEPINT(ep_idx);
	}

	/* Activate Ep */
	if (USB_EP_DIR_IS_OUT(ep)) {
		base->out_ep[ep_idx].doepctl |= USB_DWC2_DEPCTL_USBACTEP;
		usb_dw_ctrl.out_ep_ctrl[ep_idx].ep_ena = 1U;
	} else {
		base->in_ep[ep_idx].diepctl |= USB_DWC2_DEPCTL_USBACTEP;
		usb_dw_ctrl.in_ep_ctrl[ep_idx].ep_ena = 1U;
	}

	if (USB_EP_DIR_IS_OUT(ep) &&
	    usb_dw_ctrl.out_ep_ctrl[ep_idx].cb != usb_transfer_ep_callback) {
		/* Start reading now, except for transfer managed eps */
		usb_dw_prep_rx(ep, 0);
	}

	return 0;
}