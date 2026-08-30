otError otPlatRadioEnergyScan(otInstance *aInstance, uint8_t aScanChannel,
			      uint16_t aScanDuration)
{
	energy_detection_time    = aScanDuration;
	energy_detection_channel = aScanChannel;

	if (radio_api->ed_scan == NULL) {
		return OT_ERROR_NOT_IMPLEMENTED;
	}

	reset_pending_event(PENDING_EVENT_DETECT_ENERGY);
	reset_pending_event(PENDING_EVENT_DETECT_ENERGY_DONE);

	radio_api->set_channel(radio_dev, aScanChannel);

	if (radio_api->ed_scan(radio_dev, energy_detection_time, energy_detected) != 0) {
		/*
		 * OpenThread API does not accept failure of this function,
		 * it can return 'No Error' or 'Not Implemented' error only.
		 * If ed_scan start failed event is set to schedule the scan at
		 * later time.
		 */
		LOG_ERR("Failed do start energy scan, scheduling for later");
		set_pending_event(PENDING_EVENT_DETECT_ENERGY);
	}

	return OT_ERROR_NONE;
}