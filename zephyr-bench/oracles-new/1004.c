void uvc_device_init(const struct device *const dev, const struct device *const video_dev)
{
	struct uvc_data *data = dev->data;
	const struct uvc_config *cfg = dev->config;
	const struct uvc_control_map *map = NULL;
	uint32_t mask = 0;
	size_t map_sz = 0;

	if (atomic_test_and_set_bit(&data->state, UVC_STATE_INITIALIZED)) {
		LOG_WRN("UVC instance '%s' is already initialized ", dev->name);
		return;
	}

	data->video_dev = video_dev;

	/* Generate VideoControl descriptors (interface 0) */

	uvc_get_control_map(UVC_VC_INPUT_TERMINAL, &map, &map_sz);
	mask = uvc_get_mask(data->video_dev, map, map_sz);
	cfg->desc->if0_ct.bmControls[0] = mask >> 0;
	cfg->desc->if0_ct.bmControls[1] = mask >> 8;
	cfg->desc->if0_ct.bmControls[2] = mask >> 16;

	uvc_get_control_map(UVC_VC_PROCESSING_UNIT, &map, &map_sz);
	mask = uvc_get_mask(data->video_dev, map, map_sz);
	cfg->desc->if0_pu.bmControls[0] = mask >> 0;
	cfg->desc->if0_pu.bmControls[1] = mask >> 8;
	cfg->desc->if0_pu.bmControls[2] = mask >> 16;

	uvc_get_control_map(UVC_VC_EXTENSION_UNIT, &map, &map_sz);
	mask = uvc_get_mask(data->video_dev, map, map_sz);
	cfg->desc->if0_xu.bmControls[0] = mask >> 0;
	cfg->desc->if0_xu.bmControls[1] = mask >> 8;
	cfg->desc->if0_xu.bmControls[2] = mask >> 16;
	cfg->desc->if0_xu.bmControls[3] = mask >> 24;
}