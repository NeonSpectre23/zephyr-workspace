int ec_host_cmd_backend_sim_data_received(const uint8_t *buffer, size_t len)
{
	struct ec_host_cmd_sim_ctx *hc_sim = (struct ec_host_cmd_sim_ctx *)ec_host_cmd_sim.ctx;

	memcpy(hc_sim->rx_ctx->buf, buffer, len);
	hc_sim->rx_ctx->len = len;

	ec_host_cmd_rx_notify();

	return 0;
}