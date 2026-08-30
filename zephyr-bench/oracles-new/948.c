int usb_dc_ep_mps(const uint8_t ep)
{
	enum usb_dw_out_ep_idx ep_idx = USB_EP_GET_IDX(ep);

	if (!usb_dw_ctrl.attached || !usb_dw_ep_is_valid(ep)) {
		LOG_ERR("Not attached / Invalid endpoint: EP 0x%x", ep);
		return -EINVAL;
	}

	if (USB_EP_DIR_IS_OUT(ep)) {
		return usb_dw_ctrl.out_ep_ctrl[ep_idx].mps;
	} else {
		return usb_dw_ctrl.in_ep_ctrl[ep_idx].mps;
	}
}