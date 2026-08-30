void otPlatRadioSetPanId(otInstance *aInstance, uint16_t aPanId)
{
	ARG_UNUSED(aInstance);

	radio_api->filter(radio_dev, true, IEEE802154_FILTER_TYPE_PAN_ID,
			  (struct ieee802154_filter *) &aPanId);
}