otError otPlatRadioClearSrcMatchShortEntry(otInstance *aInstance,
					   const uint16_t aShortAddress)
{
	ARG_UNUSED(aInstance);

	uint8_t short_address[SHORT_ADDRESS_SIZE];
	struct ieee802154_config config = {
		.ack_fpb.enabled = false,
		.ack_fpb.addr = short_address,
		.ack_fpb.extended = false
	};

	sys_put_le16(aShortAddress, short_address);

	if (radio_api->configure(radio_dev, IEEE802154_CONFIG_ACK_FPB,
				 &config) != 0) {
		return OT_ERROR_NO_ADDRESS;
	}

	return OT_ERROR_NONE;
}