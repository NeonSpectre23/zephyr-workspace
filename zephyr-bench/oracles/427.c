int usbd_add_descriptor(struct usbd_context *const uds_ctx,
			struct usbd_desc_node *const desc_nd)
{
	struct usb_device_descriptor *hs_desc, *fs_desc;
	int ret = 0;

	usbd_device_lock(uds_ctx);

	hs_desc = uds_ctx->hs_desc;
	if (USBD_SUPPORTS_HIGH_SPEED && hs_desc == NULL) {
		ret = -EPERM;
		goto add_descriptor_error;
	}

	fs_desc = uds_ctx->fs_desc;
	if (!fs_desc || usbd_is_initialized(uds_ctx)) {
		ret = -EPERM;
		goto add_descriptor_error;
	}

	/* Check if descriptor list is initialized */
	if (!sys_dnode_is_linked(&uds_ctx->descriptors)) {
		LOG_DBG("Initialize descriptors list");
		sys_dlist_init(&uds_ctx->descriptors);
	}

	if (sys_dnode_is_linked(&desc_nd->node)) {
		ret = -EALREADY;
		goto add_descriptor_error;
	}

	if (IS_ENABLED(CONFIG_USBD_BOS_SUPPORT) &&
	    desc_nd->bDescriptorType == USB_DESC_BOS) {
		if (IS_ENABLED(CONFIG_USBD_VREQ_SUPPORT) &&
		    desc_nd->bos.utype == USBD_DUT_BOS_VREQ) {
			ret =  usbd_device_register_vreq(uds_ctx, desc_nd->bos.vreq_nd);
			if (ret) {
				goto add_descriptor_error;
			}
		}

		sys_dlist_append(&uds_ctx->descriptors, &desc_nd->node);
	}

	if (desc_nd->bDescriptorType == USB_DESC_STRING) {
		ret = desc_add_and_update_idx(uds_ctx, desc_nd);
		if (ret) {
			ret = -EINVAL;
			goto add_descriptor_error;
		}

		switch (desc_nd->str.utype) {
		case USBD_DUT_STRING_LANG:
			break;
		case USBD_DUT_STRING_MANUFACTURER:
			if (USBD_SUPPORTS_HIGH_SPEED) {
				hs_desc->iManufacturer = desc_nd->str.idx;
			}

			fs_desc->iManufacturer = desc_nd->str.idx;
			break;
		case USBD_DUT_STRING_PRODUCT:
			if (USBD_SUPPORTS_HIGH_SPEED) {
				hs_desc->iProduct = desc_nd->str.idx;
			}

			fs_desc->iProduct = desc_nd->str.idx;
			break;
		case USBD_DUT_STRING_SERIAL_NUMBER:
			if (USBD_SUPPORTS_HIGH_SPEED) {
				hs_desc->iSerialNumber = desc_nd->str.idx;
			}

			fs_desc->iSerialNumber = desc_nd->str.idx;
			break;
		default:
			break;
		}
	}

add_descriptor_error:
	usbd_device_unlock(uds_ctx);
	return ret;
}