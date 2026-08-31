uint8_t *usb_get_device_descriptor(void)
{
	static bool initialized;

	LOG_DBG("__usb_descriptor_start %p", __usb_descriptor_start);
	LOG_DBG("__usb_descriptor_end %p", __usb_descriptor_end);

	if (!initialized) {
		if (usb_fix_descriptor(__usb_descriptor_start)) {
			LOG_ERR("Failed to fixup USB descriptor");
			return NULL;
		}

		initialized = true;
	}

	return (uint8_t *) __usb_descriptor_start;
}