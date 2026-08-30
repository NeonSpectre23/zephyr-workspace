int lorawan_set_channels_mask(uint16_t *channels_mask, size_t channels_mask_size)
{
	MibRequestConfirm_t mib_req;

	if ((channels_mask == NULL) || (channels_mask_size != region_channels_mask_size)) {
		return -EINVAL;
	}

	/* Notify MAC layer of the requested channel mask. */
	mib_req.Type = MIB_CHANNELS_MASK;
	mib_req.Param.ChannelsMask = channels_mask;

	if (LoRaMacMibSetRequestConfirm(&mib_req) != LORAMAC_STATUS_OK) {
		/* Channels mask is invalid for this region. */
		return -EINVAL;
	}

	return 0;
}