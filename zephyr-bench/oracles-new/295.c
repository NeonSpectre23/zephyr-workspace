int lorawan_set_region(enum lorawan_region region)
{
	switch (region) {

#if defined(CONFIG_LORAWAN_REGION_AS923)
	case LORAWAN_REGION_AS923:
		selected_region = LORAMAC_REGION_AS923;
		region_channels_mask_size = LORAWAN_CHANNELS_MASK_SIZE_AS923;
		break;
#endif

#if defined(CONFIG_LORAWAN_REGION_AU915)
	case LORAWAN_REGION_AU915:
		selected_region = LORAMAC_REGION_AU915;
		region_channels_mask_size = LORAWAN_CHANNELS_MASK_SIZE_AU915;
		break;
#endif

#if defined(CONFIG_LORAWAN_REGION_CN470)
	case LORAWAN_REGION_CN470:
		selected_region = LORAMAC_REGION_CN470;
		region_channels_mask_size = LORAWAN_CHANNELS_MASK_SIZE_CN470;
		break;
#endif

#if defined(CONFIG_LORAWAN_REGION_CN779)
	case LORAWAN_REGION_CN779:
		selected_region = LORAMAC_REGION_CN779;
		region_channels_mask_size = LORAWAN_CHANNELS_MASK_SIZE_CN779;
		break;
#endif

#if defined(CONFIG_LORAWAN_REGION_EU433)
	case LORAWAN_REGION_EU433:
		selected_region = LORAMAC_REGION_EU433;
		region_channels_mask_size = LORAWAN_CHANNELS_MASK_SIZE_EU433;
		break;
#endif

#if defined(CONFIG_LORAWAN_REGION_EU868)
	case LORAWAN_REGION_EU868:
		selected_region = LORAMAC_REGION_EU868;
		region_channels_mask_size = LORAWAN_CHANNELS_MASK_SIZE_EU868;
		break;
#endif

#if defined(CONFIG_LORAWAN_REGION_KR920)
	case LORAWAN_REGION_KR920:
		selected_region = LORAMAC_REGION_KR920;
		region_channels_mask_size = LORAWAN_CHANNELS_MASK_SIZE_KR920;
		break;
#endif

#if defined(CONFIG_LORAWAN_REGION_IN865)
	case LORAWAN_REGION_IN865:
		selected_region = LORAMAC_REGION_IN865;
		region_channels_mask_size = LORAWAN_CHANNELS_MASK_SIZE_IN865;
		break;
#endif

#if defined(CONFIG_LORAWAN_REGION_US915)
	case LORAWAN_REGION_US915:
		selected_region = LORAMAC_REGION_US915;
		region_channels_mask_size = LORAWAN_CHANNELS_MASK_SIZE_US915;
		break;
#endif

#if defined(CONFIG_LORAWAN_REGION_RU864)
	case LORAWAN_REGION_RU864:
		selected_region = LORAMAC_REGION_RU864;
		region_channels_mask_size = LORAWAN_CHANNELS_MASK_SIZE_RU864;
		break;
#endif

	default:
		LOG_ERR("No support for region %d!", region);
		return -ENOTSUP;
	}

	LOG_DBG("Selected region %d", region);

	return 0;
}