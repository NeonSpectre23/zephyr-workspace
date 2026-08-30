int usbd_unregister_all_classes(struct usbd_context *const uds_ctx,
				const enum usbd_speed speed, const uint8_t cfg)
{
	int ret;

	if (USBD_SUPPORTS_HIGH_SPEED && speed == USBD_SPEED_HS) {
		STRUCT_SECTION_FOREACH_ALTERNATE(usbd_class_hs, usbd_class_node, c_nd) {
			ret = usbd_unregister_class(uds_ctx, c_nd->c_data->name,
						    speed, cfg);
			if (ret) {
				LOG_ERR("Failed to unregister %s to HS configuration %u",
					c_nd->c_data->name, cfg);
				return ret;
			}
		}

		return 0;
	}

	if (speed == USBD_SPEED_FS) {
		STRUCT_SECTION_FOREACH_ALTERNATE(usbd_class_fs, usbd_class_node, c_nd) {
			ret = usbd_unregister_class(uds_ctx, c_nd->c_data->name,
						    speed, cfg);
			if (ret) {
				LOG_ERR("Failed to unregister %s to FS configuration %u",
					c_nd->c_data->name, cfg);
				return ret;
			}
		}

		return 0;
	}

	return -ENOTSUP;
}