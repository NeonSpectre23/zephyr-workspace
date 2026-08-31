int usbd_add_configuration(struct usbd_context *const uds_ctx,
			   const enum usbd_speed speed,
			   struct usbd_config_node *const cfg_nd)
{
	struct usb_cfg_descriptor *desc = cfg_nd->desc;
	sys_slist_t *configs;
	sys_snode_t *node;
	int ret = 0;

	usbd_device_lock(uds_ctx);

	if (usbd_is_initialized(uds_ctx)) {
		LOG_ERR("USB device support is initialized");
		ret = -EBUSY;
		goto add_configuration_exit;
	}

	if (speed == USBD_SPEED_HS && !USBD_SUPPORTS_HIGH_SPEED) {
		LOG_ERR("Stack was compiled without High-Speed support");
		ret = -ENOTSUP;
		goto add_configuration_exit;
	}

	if (speed == USBD_SPEED_HS &&
	    usbd_caps_speed(uds_ctx) == USBD_SPEED_FS) {
		LOG_ERR("Controller doesn't support HS");
		ret = -ENOTSUP;
		goto add_configuration_exit;
	}

	if (desc->bmAttributes & USB_SCD_REMOTE_WAKEUP) {
		struct udc_device_caps caps = udc_caps(uds_ctx->dev);

		if (!caps.rwup) {
			LOG_ERR("Feature not supported by controller");
			ret = -ENOTSUP;
			goto add_configuration_exit;
		}
	}

	configs = usbd_configs(uds_ctx, speed);
	switch (speed) {
	case USBD_SPEED_HS:
		SYS_SLIST_FOR_EACH_NODE(&uds_ctx->fs_configs, node) {
			if (node == &cfg_nd->node) {
				LOG_ERR("HS config already on FS list");
				ret = -EINVAL;
				goto add_configuration_exit;
			}
		}
		break;
	case USBD_SPEED_FS:
		SYS_SLIST_FOR_EACH_NODE(&uds_ctx->hs_configs, node) {
			if (node == &cfg_nd->node) {
				LOG_ERR("FS config already on HS list");
				ret = -EINVAL;
				goto add_configuration_exit;
			}
		}
		break;
	default:
		LOG_ERR("Unsupported configuration speed");
		ret = -ENOTSUP;
		goto add_configuration_exit;
	}

	if (sys_slist_find_and_remove(configs, &cfg_nd->node)) {
		LOG_WRN("Configuration %u re-inserted",
			usbd_config_get_value(cfg_nd));
	} else {
		uint8_t num = usbd_get_num_configs(uds_ctx, speed) + 1;

		usbd_config_set_value(cfg_nd, num);
		usbd_set_num_configs(uds_ctx, speed, num);
	}

	if (cfg_nd->str_desc_nd != NULL) {
		ret = usbd_add_descriptor(uds_ctx, cfg_nd->str_desc_nd);
		if (ret != 0) {
			LOG_ERR("Failed to add configuration string descriptor");
			goto add_configuration_exit;
		}

		desc->iConfiguration = usbd_str_desc_get_idx(cfg_nd->str_desc_nd);
	}

	sys_slist_append(configs, &cfg_nd->node);

add_configuration_exit:
	usbd_device_unlock(uds_ctx);
	return ret;
}