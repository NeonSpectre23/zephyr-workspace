int uvc_device_enable(const struct device *const dev)
{
	const struct uvc_config *cfg = dev->config;
	struct uvc_data *data = dev->data;
	int ret;

	if (!atomic_test_bit(&data->state, UVC_STATE_INITIALIZED)) {
		LOG_ERR("UVC instance '%s' is not initialized ", dev->name);
		return -EIO;
	}

	cfg->desc->if1_hdr.wTotalLength += cfg->desc->if1_color.bLength;

	ret = uvc_assign_desc(dev, &cfg->desc->if1_color, true, true);
	if (ret != 0) {
		return ret;
	}

	ret = uvc_assign_desc(dev, &cfg->desc->if1_ep_fs, true, false);
	if (ret != 0) {
		return ret;
	}

	ret = uvc_assign_desc(dev, &cfg->desc->if1_ep_hs, false, true);
	if (ret != 0) {
		return ret;
	}

	cfg->desc->if1_hdr.wTotalLength = sys_cpu_to_le16(cfg->desc->if1_hdr.wTotalLength);

	/* Generating the default probe message now that descriptors are complete */
	ret = uvc_get_vs_probe_struct(dev, &data->default_probe, UVC_GET_CUR);
	if (ret != 0) {
		LOG_ERR("init: failed to query the default probe");
		return ret;
	}

	return 0;
}