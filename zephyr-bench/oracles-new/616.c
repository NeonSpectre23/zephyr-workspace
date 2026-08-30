void otPlatRadioUpdateCslSampleTime(otInstance *aInstance, uint32_t aCslSampleTime)
{
	ARG_UNUSED(aInstance);

	/* CSL sample time points to "start of MAC" while the expected RX time
	 * refers to "end of SFD".
	 */
	struct ieee802154_config config = {
		.expected_rx_time =
			convert_32bit_us_wrapped_to_64bit_ns(aCslSampleTime - PHR_DURATION_US),
	};

	(void)radio_api->configure(radio_dev, IEEE802154_CONFIG_EXPECTED_RX_TIME, &config);
}