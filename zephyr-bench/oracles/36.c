void ec_host_cmd_backend_sim_install_send_cb(ec_host_cmd_backend_api_send cb,
					     struct ec_host_cmd_tx_buf **tx_buf)
{
	struct ec_host_cmd_sim_ctx *hc_sim = (struct ec_host_cmd_sim_ctx *)ec_host_cmd_sim.ctx;
	*tx_buf = hc_sim->tx;
	tx = cb;
}