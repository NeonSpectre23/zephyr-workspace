int lorawan_join(const struct lorawan_join_config *join_cfg)
{
	MibRequestConfirm_t mib_req;
	LoRaMacStatus_t status;
	int ret = 0;

	k_mutex_lock(&lorawan_join_mutex, K_FOREVER);

	/* MIB_PUBLIC_NETWORK powers on the radio and does not turn it off */
	mib_req.Type = MIB_PUBLIC_NETWORK;
	mib_req.Param.EnablePublicNetwork = IS_ENABLED(CONFIG_LORAWAN_PUBLIC_NETWORK);
	LoRaMacMibSetRequestConfirm(&mib_req);

	if (join_cfg->mode == LORAWAN_ACT_OTAA) {
		status = lorawan_join_otaa(join_cfg);
		if (status != LORAMAC_STATUS_OK) {
			LOG_ERR("OTAA join failed: %s",
				lorawan_status2str(status));
			ret = lorawan_status2errno(status);
			goto out;
		}

		LOG_DBG("Network join request sent!");

		/*
		 * We can be sure that the semaphore will be released for
		 * both success and failure cases after a specific time period.
		 * So we can use K_FOREVER and no need to check the return val.
		 */
		k_sem_take(&mlme_confirm_sem, K_FOREVER);
		if (last_mlme_confirm_status != LORAMAC_EVENT_INFO_STATUS_OK) {
			ret = lorawan_eventinfo2errno(last_mlme_confirm_status);
			goto out;
		}
	} else if (join_cfg->mode == LORAWAN_ACT_ABP) {
		status = lorawan_join_abp(join_cfg);
		if (status != LORAMAC_STATUS_OK) {
			LOG_ERR("ABP join failed: %s",
				lorawan_status2str(status));
			ret = lorawan_status2errno(status);
			goto out;
		}
	} else {
		ret = -EINVAL;
	}

out:
	/* If the join succeeded */
	if (ret == 0) {
		/*
		 * Several regions (AS923, AU915, US915) overwrite the
		 * datarate as part of the join process. Reset the datarate
		 * to the value requested (and validated) in
		 * lorawan_set_datarate so that the MAC layer is aware of the
		 * set datarate for LoRaMacQueryTxPossible. This is only
		 * performed when ADR is disabled as it the network servers
		 * responsibility to increase datarates when ADR is enabled.
		 */
		if (!atomic_test_bit(lorawan_flags, LORAWAN_FLAG_ADR_ENABLE)) {
			MibRequestConfirm_t mib_req2;

			mib_req2.Type = MIB_CHANNELS_DATARATE;
			mib_req2.Param.ChannelsDatarate = default_datarate;
			LoRaMacMibSetRequestConfirm(&mib_req2);
		}

		/*
		 * Force a notification of the datarate on network join as the
		 * user may not have explicitly set a datarate to use.
		 */
		datarate_observe(true);
	}

	k_mutex_unlock(&lorawan_join_mutex);
	return ret;
}