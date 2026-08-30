void otPlatRadioEnableSrcMatch(otInstance *aInstance, bool aEnable)
{
	ARG_UNUSED(aInstance);

	struct ieee802154_config config = {
		.auto_ack_fpb.enabled = aEnable,
		.auto_ack_fpb.mode = IEEE802154_FPB_ADDR_MATCH_THREAD,
	};

	(void)radio_api->configure(radio_dev, IEEE802154_CONFIG_AUTO_ACK_FPB,
				   &config);
}