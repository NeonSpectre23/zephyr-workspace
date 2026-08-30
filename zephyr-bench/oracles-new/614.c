otError otPlatRadioSleep(otInstance *aInstance)
{
	ARG_UNUSED(aInstance);

	if (sState != OT_RADIO_STATE_SLEEP && sState != OT_RADIO_STATE_RECEIVE) {
		return OT_ERROR_INVALID_STATE;
	}

	radio_api->stop(radio_dev);
	sState = OT_RADIO_STATE_SLEEP;

	return OT_ERROR_NONE;
}