bool otPlatRadioIsEnabled(otInstance *aInstance)
{
	ARG_UNUSED(aInstance);

	return (sState != OT_RADIO_STATE_DISABLED) ? true : false;
}