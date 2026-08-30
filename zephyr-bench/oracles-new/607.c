otError otPlatRadioReceive(otInstance *aInstance, uint8_t aChannel)
{
	ARG_UNUSED(aInstance);

	if (sState == OT_RADIO_STATE_DISABLED) {
		return OT_ERROR_INVALID_STATE;
	}

	channel = aChannel;

	radio_api->set_channel(radio_dev, aChannel);
	radio_api->set_txpower(radio_dev, get_transmit_power_for_channel(channel));
	radio_api->start(radio_dev);
	sState = OT_RADIO_STATE_RECEIVE;

	if (is_pending_event_set(PENDING_EVENT_TX_DONE)) {
		reset_pending_event(PENDING_EVENT_TX_DONE);
	}

	return OT_ERROR_NONE;
}