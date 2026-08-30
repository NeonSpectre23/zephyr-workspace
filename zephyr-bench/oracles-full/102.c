static uint8_t init(uint8_t chan, uint8_t phy, int8_t tx_power,
		    bool cte, void (*isr)(void *))
{
	int err;
	uint8_t ret;

	if (started) {
		return BT_HCI_ERR_CMD_DISALLOWED;
	}

	/* start coarse timer */
	cntr_start();

	/* Setup resources required by Radio */
	err = lll_hfclock_on_wait();
	LL_ASSERT_ERR(err >= 0);

	/* Reset Radio h/w */
	radio_reset();
	radio_isr_set(isr, NULL);

#if defined(CONFIG_BT_CTLR_DF)
	/* Reset  Radio DF */
	radio_df_reset();
#endif

	/* Store value needed in Tx/Rx ISR */
	if (phy < BT_HCI_LE_TX_PHY_CODED_S2) {
		test_phy = BIT(phy - 1);
		test_phy_flags = 1U;
	} else {
		test_phy = BIT(2);
		test_phy_flags = 0U;
	}

	/* Setup Radio in Tx/Rx */
	/* NOTE: No whitening in test mode. */
	radio_phy_set(test_phy, test_phy_flags);

	ret = tx_power_set(tx_power);

	radio_freq_chan_set((chan << 1) + 2);
	radio_aa_set((uint8_t *)&test_sync_word);
	radio_crc_configure(0x65b, PDU_AC_CRC_IV);
	radio_pkt_configure(RADIO_PKT_CONF_LENGTH_8BIT, PDU_DTM_PAYLOAD_SIZE_MAX,
			    RADIO_PKT_CONF_PHY(test_phy) |
			    RADIO_PKT_CONF_PDU_TYPE(IS_ENABLED(CONFIG_BT_CTLR_DF_CTE_TX) ?
								RADIO_PKT_CONF_PDU_TYPE_DC :
								RADIO_PKT_CONF_PDU_TYPE_AC) |
			    RADIO_PKT_CONF_CTE(cte ? RADIO_PKT_CONF_CTE_ENABLED :
						     RADIO_PKT_CONF_CTE_DISABLED));

	return ret;
}