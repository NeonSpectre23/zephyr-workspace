struct kbc_reg *const inst_kbc = host_sub_cfg.inst_kbc;

	/* Make sure the previous OBF and IRQ has been sent out. */
	k_busy_wait(4);
	/* Set FW_OBF to clear OBF flag in both STATUS and HIKMST to 0 */
	inst_kbc->HICTRL |= BIT(NPCX_HICTRL_FW_OBF);
	/* Ensure there is no OBF set in this period. */
	k_busy_wait(4);

	/*
	 * Init KBC with:
	 * 1. Enable Input Buffer Full (IBF) core interrupt for Keyboard/mouse.
	 * 2. Enable Output Buffer Full Mouse(OBFM) SIRQ 12.
	 * 3. Enable Output Buffer Full Keyboard (OBFK) SIRQ 1.
	 */
	if (IS_ENABLED(CONFIG_ESPI_NPCX_i8042_KBC_AUX_VWIRE_IRQ_WORKAROUND)) {
		inst_kbc->HICTRL = BIT(NPCX_HICTRL_IBFCIE);
	} else {
		inst_kbc->HICTRL =
			BIT(NPCX_HICTRL_IBFCIE) | BIT(NPCX_HICTRL_OBFMIE) | BIT(NPCX_HICTRL_OBFKIE);
	}
	/* Configure SIRQ 1/12 type (level + high) */
	inst_kbc->HIIRQC = 0x00;
}
#endif

/* Host ACPI sub-device loc