otError otPlatRadioTransmit(otInstance *aInstance, otRadioFrame *aPacket)
{
	otError error = OT_ERROR_INVALID_STATE;

	ARG_UNUSED(aInstance);
	ARG_UNUSED(aPacket);

	__ASSERT_NO_MSG(aPacket == &sTransmitFrame);

	enum ieee802154_hw_caps radio_caps;

	radio_caps = radio_api->get_capabilities(radio_dev);

	if (sState == OT_RADIO_STATE_RECEIVE ||
	    (sState == OT_RADIO_STATE_SLEEP &&
	     radio_caps & IEEE802154_HW_SLEEP_TO_TX)) {
		if (run_tx_task(aInstance) == 0) {
			error = OT_ERROR_NONE;
		}
	}

	return error;
}