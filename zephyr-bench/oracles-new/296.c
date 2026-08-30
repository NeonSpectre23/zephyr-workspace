int lorawan_start(void)
{
	LoRaMacStatus_t status;
	MibRequestConfirm_t mib_req;
	GetPhyParams_t phy_params;
	PhyParam_t phy_param;

	status = LoRaMacInitialization(&mac_primitives, &mac_callbacks,
				       selected_region);
	if (status != LORAMAC_STATUS_OK) {
		LOG_ERR("LoRaMacInitialization failed: %s",
			lorawan_status2str(status));
		return -EINVAL;
	}

	LOG_DBG("LoRaMAC Initialized");

	if (!IS_ENABLED(CONFIG_LORAWAN_NVM_NONE)) {
		lorawan_nvm_init();
		lorawan_nvm_data_restore();
	}

	status = LoRaMacStart();
	if (status != LORAMAC_STATUS_OK) {
		LOG_ERR("Failed to start the LoRaMAC stack: %s",
			lorawan_status2str(status));
		return -EINVAL;
	}

	/* Retrieve the default TX datarate for selected region */
	phy_params.Attribute = PHY_DEF_TX_DR;
	phy_param = RegionGetPhyParam(selected_region, &phy_params);
	default_datarate = phy_param.Value;
	current_datarate = default_datarate;

	/* TODO: Move these to a proper location */
	mib_req.Type = MIB_SYSTEM_MAX_RX_ERROR;
	mib_req.Param.SystemMaxRxError = CONFIG_LORAWAN_SYSTEM_MAX_RX_ERROR;
	LoRaMacMibSetRequestConfirm(&mib_req);

	return 0;
}