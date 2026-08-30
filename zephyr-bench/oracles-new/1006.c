static int8_t verify_image(const struct device *dev)
{
	const struct mpfs_fpga_config *cfg = dev->config;
	int8_t status = EINVAL;
	uint32_t value = 0;

	LOG_INF("Image verification started...");

	/* Once system controller starts processing command The busy bit will
	 * go 1. Make sure that service is complete i.e. BUSY bit is gone 0
	 */
	while (scb_read(cfg->base, SERVICES_SR_OFFSET) & SCBCTRL_SERVICESSR_BUSY_MASK) {
		;
	}

	/* Form the SS command: bit 0 to 6 is the opcode, bit 7 to 15 is the Mailbox
	 * offset For some services this field has another meaning.
	 * (e.g. for IAP bit-stream auth. it means spi_idx)
	 */
	scb_write(cfg->mailbox, 0, 0x1500400);

	value = (MSS_SYS_BITSTREAM_AUTHENTICATE_CMD << 16) | 0x1;
	scb_write(cfg->base, SERVICES_CR_OFFSET, value);

	/* REQ bit will remain set till the system controller starts
	 * processing command. Since DRI is slow interface, we are waiting
	 * here to make sure System controller has started processing
	 * command
	 */
	while (scb_read(cfg->base, SERVICES_CR_OFFSET) & SCBCTRL_SERVICESCR_REQ_MASK) {
		;
	}

	/* Once system controller starts processing command The busy bit will
	 * go 1. Make sure that service is complete i.e. BUSY bit is gone 0
	 */
	while (scb_read(cfg->base, SERVICES_SR_OFFSET) & SCBCTRL_SERVICESSR_BUSY_MASK) {
		;
	}

	/* Read the status returned by System Controller */
	status = ((scb_read(cfg->base, SERVICES_SR_OFFSET) & SCBCTRL_SERVICESSR_STATUS_MASK) >>
		  SCBCTRL_SERVICESSR_STATUS);
	LOG_INF("Image verification status  : %x   ", status);

	return status;
}