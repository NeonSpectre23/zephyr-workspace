int usb_dc_ep_disable(const uint8_t ep)
{
	struct usb_dwc2_reg *const base = usb_dw_cfg.base;
	uint8_t ep_idx = USB_EP_GET_IDX(ep);

	if (!usb_dw_ctrl.attached || !usb_dw_ep_is_valid(ep)) {
		LOG_ERR("Not attached / Invalid endpoint: EP 0x%x", ep);
		return -EINVAL;
	}

	/* Disable EP interrupts */
	if (USB_EP_DIR_IS_OUT(ep)) {
		base->daintmsk &= ~USB_DWC2_DAINT_OUTEPINT(ep_idx);
		base->doepmsk &= ~USB_DWC2_DOEPINT_SETUP;
	} else {
		base->daintmsk &= ~USB_DWC2_DAINT_INEPINT(ep_idx);
		base->diepmsk &= ~USB_DWC2_DIEPINT_XFERCOMPL;
		base->gintmsk &= ~USB_DWC2_GINTSTS_RXFLVL;
	}

	/* De-activate, disable and set NAK for Ep */
	if (USB_EP_DIR_IS_OUT(ep)) {
		base->out_ep[ep_idx].doepctl &=
		    ~(USB_DWC2_DEPCTL_USBACTEP |
		    USB_DWC2_DEPCTL_EPENA |
		    USB_DWC2_DEPCTL_SNAK);
		usb_dw_ctrl.out_ep_ctrl[ep_idx].ep_ena = 0U;
	} else {
		base->in_ep[ep_idx].diepctl &=
		    ~(USB_DWC2_DEPCTL_USBACTEP |
		    USB_DWC2_DEPCTL_EPENA |
		    USB_DWC2_DEPCTL_SNAK);
		usb_dw_ctrl.in_ep_ctrl[ep_idx].ep_ena = 0U;
	}

	return 0;
}