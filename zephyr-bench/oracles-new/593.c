otError otPlatRadioClearSrcMatchExtEntry(otInstance *aInstance,
					 const otExtAddress *aExtAddress)
{
	ARG_UNUSED(aInstance);

	struct ieee802154_config config = {
		.ack_fpb.enabled = false,
		.ack_fpb.addr = (uint8_t *)aExtAddress->m8,
		.ack_fpb.extended = true
	};

	if (radio_api->configure(radio_dev, IEEE802154_CONFIG_ACK_FPB,
				 &config) != 0) {
		return OT_ERROR_NO_ADDRESS;
	}

	return OT_ERROR_NONE;
}