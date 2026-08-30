int usb_write(uint8_t ep, const uint8_t *data, uint32_t data_len, uint32_t *bytes_ret)
{
	int tries = CONFIG_USB_NUMOF_EP_WRITE_RETRIES;
	int ret;

	do {
		ret = usb_dc_ep_write(ep, data, data_len, bytes_ret);
		if (ret == -EAGAIN) {
			LOG_WRN("Failed to write endpoint buffer 0x%02x", ep);
			k_yield();
		}

	} while (ret == -EAGAIN && tries--);

	return ret;
}