static void end(struct bt_mesh_blob_cli *cli, bool success)
{
	const struct bt_mesh_blob_xfer *xfer = cli->xfer;

	LOG_DBG("%u", success);

	io_close(cli);
	cli_state_reset(cli);
	if (cli->cb && cli->cb->end) {
		cli->cb->end(cli, xfer, success);
	}
}