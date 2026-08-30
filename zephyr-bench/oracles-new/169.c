static void ibecc_errsts_clear(const pcie_bdf_t bdf)
{
	uint32_t errsts;

	errsts = pcie_conf_read(bdf, ERRSTS_REG);

	if ((errsts & (ERRSTS_IBECC_COR | ERRSTS_IBECC_UC)) == 0) {
		return;
	}

	pcie_conf_write(bdf, ERRSTS_REG, errsts);
}