otError otPlatRadioEnable(otInstance *aInstance)
{
	ARG_UNUSED(aInstance);

	if (sState != OT_RADIO_STATE_DISABLED && sState != OT_RADIO_STATE_SLEEP) {
		return OT_ERROR_INVALID_STATE;
	}

	sState = OT_RADIO_STATE_SLEEP;
	return OT_ERROR_NONE;
}