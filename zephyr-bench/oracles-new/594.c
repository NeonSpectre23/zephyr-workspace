void otPlatRadioClearSrcMatchShortEntries(otInstance *aInstance)
{
	ARG_UNUSED(aInstance);

	struct ieee802154_config config = {
		.ack_fpb.enabled = false,
		.ack_fpb.addr = NULL,
		.ack_fpb.extended = false
	};

	(void)radio_api->configure(radio_dev, IEEE802154_CONFIG_ACK_FPB,
				   &config);
}