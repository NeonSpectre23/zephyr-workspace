int8_t otPlatRadioGetRssi(otInstance *aInstance)
{
	int8_t ret_rssi = INT8_MAX;
	int error = 0;
	const uint16_t detection_time = 1;
	enum ieee802154_hw_caps radio_caps;
	ARG_UNUSED(aInstance);

	radio_caps = radio_api->get_capabilities(radio_dev);

	if (!(radio_caps & IEEE802154_HW_ENERGY_SCAN)) {
		/*
		 * TODO: No API in Zephyr to get the RSSI
		 * when IEEE802154_HW_ENERGY_SCAN is not available
		 */
		ret_rssi = 0;
	} else {
		/*
		 * Blocking implementation of get RSSI
		 * using no-blocking ed_scan
		 */
		error = radio_api->ed_scan(radio_dev, detection_time,
					   get_rssi_energy_detected);

		if (error == 0) {
			k_sem_take(&radio_sem, K_FOREVER);

			ret_rssi = (int8_t)energy_detected_value;
		}
	}

	return ret_rssi;
}