void otPlatRadioSetPromiscuous(otInstance *aInstance, bool aEnable)
{
	struct ieee802154_config config = {
		.promiscuous = aEnable
	};

	ARG_UNUSED(aInstance);

	LOG_DBG("PromiscuousMode=%d", aEnable ? 1 : 0);

	promiscuous = aEnable;
	/* TODO: Should check whether the radio driver actually supports
	 *       promiscuous mode, see net_if_l2(iface)->get_flags() and
	 *       ieee802154_radio_get_hw_capabilities(iface).
	 */
	radio_api->configure(radio_dev, IEEE802154_CONFIG_PROMISCUOUS, &config);
}