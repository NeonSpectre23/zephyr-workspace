otError otPlatRadioReceiveAt(otInstance *aInstance, uint8_t aChannel,
			     uint32_t aStart, uint32_t aDuration)
{
	int result;

	ARG_UNUSED(aInstance);

	struct ieee802154_config config = {
		.rx_slot.channel = aChannel,
		.rx_slot.start = convert_32bit_us_wrapped_to_64bit_ns(aStart),
		.rx_slot.duration = (net_time_t)aDuration * NSEC_PER_USEC,
	};

	result = radio_api->configure(radio_dev, IEEE802154_CONFIG_RX_SLOT,
				      &config);

	return result ? OT_ERROR_FAILED : OT_ERROR_NONE;
}