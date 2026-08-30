void platformRadioProcess(otInstance *aInstance)
{
	bool event_pending = false;

	if (is_pending_event_set(PENDING_EVENT_FRAME_TO_SEND)) {
		struct net_pkt *evt_pkt;

		reset_pending_event(PENDING_EVENT_FRAME_TO_SEND);
		while ((evt_pkt = (struct net_pkt *) k_fifo_get(&tx_pkt_fifo, K_NO_WAIT)) != NULL) {
			if (IS_ENABLED(CONFIG_OPENTHREAD_COPROCESSOR_RCP)) {
				net_pkt_unref(evt_pkt);
			} else {
				openthread_handle_frame_to_send(aInstance, evt_pkt);
			}
		}
	}

	if (is_pending_event_set(PENDING_EVENT_FRAME_RECEIVED)) {
		struct net_pkt *rx_pkt;

		reset_pending_event(PENDING_EVENT_FRAME_RECEIVED);
		while ((rx_pkt = (struct net_pkt *) k_fifo_get(&rx_pkt_fifo, K_NO_WAIT)) != NULL) {
			openthread_handle_received_frame(aInstance, rx_pkt);
		}
	}

	if (is_pending_event_set(PENDING_EVENT_RX_FAILED)) {
		reset_pending_event(PENDING_EVENT_RX_FAILED);
		if (IS_ENABLED(CONFIG_OPENTHREAD_DIAG) && otPlatDiagModeGet()) {
			otPlatDiagRadioReceiveDone(aInstance, NULL, rx_result);
		} else {
			otPlatRadioReceiveDone(aInstance, NULL, rx_result);
		}
	}

	if (is_pending_event_set(PENDING_EVENT_TX_STARTED)) {
		reset_pending_event(PENDING_EVENT_TX_STARTED);
		otPlatRadioTxStarted(aInstance, &sTransmitFrame);
	}

	if (is_pending_event_set(PENDING_EVENT_TX_DONE)) {
		reset_pending_event(PENDING_EVENT_TX_DONE);

		if (sState == OT_RADIO_STATE_TRANSMIT ||
		    radio_api->get_capabilities(radio_dev) & IEEE802154_HW_SLEEP_TO_TX) {
			sState = OT_RADIO_STATE_RECEIVE;
			handle_tx_done(aInstance);
		}
	}

	if (is_pending_event_set(PENDING_EVENT_SLEEP)) {
		reset_pending_event(PENDING_EVENT_SLEEP);
		ARG_UNUSED(otPlatRadioSleep(aInstance));
	}

	/* handle events that can't run during transmission */
	if (sState != OT_RADIO_STATE_TRANSMIT) {
		if (is_pending_event_set(PENDING_EVENT_DETECT_ENERGY)) {
			radio_api->set_channel(radio_dev,
					       energy_detection_channel);

			if (!radio_api->ed_scan(radio_dev,
						energy_detection_time,
						energy_detected)) {
				reset_pending_event(
					PENDING_EVENT_DETECT_ENERGY);
			} else {
				event_pending = true;
			}
		}

		if (is_pending_event_set(PENDING_EVENT_DETECT_ENERGY_DONE)) {
			otPlatRadioEnergyScanDone(aInstance, (int8_t) energy_detected_value);
			reset_pending_event(PENDING_EVENT_DETECT_ENERGY_DONE);
		}
	}

	if (event_pending) {
		otSysEventSignalPending();
	}
}