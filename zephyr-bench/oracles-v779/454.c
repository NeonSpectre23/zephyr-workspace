static struct usb_cfg_data *usb_get_cfg_data(struct usb_if_descriptor *iface)
{
	STRUCT_SECTION_FOREACH(usb_cfg_data, cfg_data) {
		if (cfg_data->interface_descriptor == iface) {
			return cfg_data;
		}
	}

	return NULL;
}