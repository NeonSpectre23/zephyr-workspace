static int edac_ibecc_init(const struct device *dev)
{
	const pcie_bdf_t bdf = PCI_HOST_BRIDGE;
	struct ibecc_data *data = dev->data;
	uint64_t mchbar;
	uint32_t conf_data;

	conf_data = pcie_conf_read(bdf, PCIE_CONF_ID);
	switch (conf_data) {
	case PCIE_ID(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_SKU5):
		__fallthrough;
	case PCIE_ID(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_SKU6):
		__fallthrough;
	case PCIE_ID(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_SKU7):
		__fallthrough;
	case PCIE_ID(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_SKU8):
		__fallthrough;
	case PCIE_ID(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_SKU9):
		__fallthrough;
	case PCIE_ID(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_SKU10):
		__fallthrough;
	case PCIE_ID(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_SKU11):
		__fallthrough;
	case PCIE_ID(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_SKU12):
		__fallthrough;
	case PCIE_ID(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_SKU13):
		__fallthrough;
	case PCIE_ID(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_SKU14):
		__fallthrough;
	case PCIE_ID(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_SKU15):
		break;
	default:
		LOG_ERR("PCI Probe failed"); /* LCOV_EXCL_BR_LINE */
		return -ENODEV;
	}

	if (!ibecc_enabled(bdf)) {
		LOG_ERR("IBECC is not enabled"); /* LCOV_EXCL_BR_LINE */
		return -ENODEV;
	}

	mchbar = pcie_conf_read(bdf, MCHBAR_REG);
	mchbar |= (uint64_t)pcie_conf_read(bdf, MCHBAR_REG + 1) << 32;

	/* Check that MCHBAR is enabled */
	if ((mchbar & MCHBAR_ENABLE) == 0) {
		LOG_ERR("MCHBAR is not enabled"); /* LCOV_EXCL_BR_LINE */
		return -ENODEV;
	}

	mchbar &= MCHBAR_MASK;

	device_map(&data->mchbar, mchbar, MCH_SIZE, K_MEM_CACHE_NONE);

	/* Enable Host Bridge generated SERR event */
	ibecc_errcmd_setup(bdf, true);

	LOG_INF("IBECC driver initialized"); /* LCOV_EXCL_BR_LINE */

	return 0;
}