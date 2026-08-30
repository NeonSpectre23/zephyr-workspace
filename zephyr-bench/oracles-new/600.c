otRadioCaps otPlatRadioGetCaps(otInstance *aInstance)
{
	otRadioCaps caps = OT_RADIO_CAPS_NONE;

	enum ieee802154_hw_caps radio_caps;
	ARG_UNUSED(aInstance);
	__ASSERT(radio_api,
	    "platformRadioInit needs to be called prior to otPlatRadioGetCaps");

	radio_caps = radio_api->get_capabilities(radio_dev);

	if (radio_caps & IEEE802154_HW_ENERGY_SCAN) {
		caps |= OT_RADIO_CAPS_ENERGY_SCAN;
	}

	if (radio_caps & IEEE802154_HW_CSMA) {
		caps |= OT_RADIO_CAPS_CSMA_BACKOFF;
	}

	if (radio_caps & IEEE802154_HW_TX_RX_ACK) {
		caps |= OT_RADIO_CAPS_ACK_TIMEOUT;
	}

	if (radio_caps & IEEE802154_HW_SLEEP_TO_TX) {
		caps |= OT_RADIO_CAPS_SLEEP_TO_TX;
	}

#if !defined(CONFIG_OPENTHREAD_THREAD_VERSION_1_1)
	if (radio_caps & IEEE802154_HW_TX_SEC) {
		caps |= OT_RADIO_CAPS_TRANSMIT_SEC;
	}
#endif

#if defined(CONFIG_OPENTHREAD_PLATFORM_PKT_TXTIME)
	if (radio_caps & IEEE802154_HW_TXTIME) {
		caps |= OT_RADIO_CAPS_TRANSMIT_TIMING;
	}
#endif

	if (radio_caps & IEEE802154_HW_RXTIME) {
		caps |= OT_RADIO_CAPS_RECEIVE_TIMING;
	}

	if (radio_caps & IEEE802154_RX_ON_WHEN_IDLE) {
		caps |= OT_RADIO_CAPS_RX_ON_WHEN_IDLE;
	}

	return caps;
}