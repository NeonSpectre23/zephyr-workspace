int uvc_device_add_format(const struct device *const dev, const struct video_format *const fmt)
{
	struct uvc_data *data = dev->data;
	const struct uvc_config *cfg = dev->config;
	int ret;

	if (!atomic_test_bit(&data->state, UVC_STATE_INITIALIZED)) {
		LOG_ERR("UVC instance '%s' is not initialized ", dev->name);
		return -EIO;
	}

	if (data->video_dev == NULL) {
		LOG_ERR("Video device not yet configured into UVC");
		return -EINVAL;
	}

	if (fmt->size == 0) {
		LOG_ERR("The format size must be set prior to add it to UVC");
		return -EINVAL;
	}

	if (data->last_pix_fmt != fmt->pixelformat &&
	    data->fmt_desc_idx + 2 > CONFIG_USBD_VIDEO_MAX_FORMATS) {
		LOG_WRN("Not enough format descriptors to add descriptors for '%s' and %ux%u",
			VIDEO_FOURCC_TO_STR(fmt->pixelformat), fmt->width, fmt->height);
		return -ENOMEM;
	}

	if (data->last_pix_fmt == fmt->pixelformat &&
	    data->fmt_desc_idx + 1 > CONFIG_USBD_VIDEO_MAX_FORMATS) {
		LOG_WRN("Not enough format descriptors to add descriptors %ux%u",
			fmt->width, fmt->height);
		return -ENOMEM;
	}

	if (data->last_pix_fmt != fmt->pixelformat) {
		if (data->last_pix_fmt != 0) {
			cfg->desc->if1_hdr.wTotalLength += cfg->desc->if1_color.bLength;

			ret = uvc_assign_desc(dev, &cfg->desc->if1_color, true, true);
			if (ret != 0) {
				return ret;
			}
		}

		ret = uvc_add_vs_format_desc(dev, &data->last_format_desc, fmt->pixelformat);
		if (ret != 0) {
			return ret;
		}
	}

	ret = uvc_add_vs_frame_desc(dev, data->last_format_desc, fmt);
	if (ret != 0) {
		return ret;
	}

	data->last_pix_fmt = fmt->pixelformat;

	return 0;
}