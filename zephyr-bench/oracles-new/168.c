static void ibecc_errcmd_setup(const pcie_bdf_t bdf, bool enable)
{
	uint32_t errcmd;

	errcmd = pcie_conf_read(bdf, ERRCMD_REG);

	if (enable) {
		errcmd |= (ERRCMD_IBECC_COR | ERRCMD_IBECC_UC) << 16;
	} else {
		errcmd &= ~(ERRCMD_IBECC_COR | ERRCMD_IBECC_UC) << 16;
	}

	pcie_conf_write(bdf, ERRCMD_REG, errcmd);
}