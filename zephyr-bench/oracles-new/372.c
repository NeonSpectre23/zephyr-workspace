int modem_ubx_attach(struct modem_ubx *ubx, struct modem_pipe *pipe)
{
	if (atomic_test_and_set_bit(&ubx->attached, 0) == true) {
		return 0;
	}

	ubx->pipe = pipe;
	modem_pipe_attach(ubx->pipe, modem_ubx_pipe_callback, ubx);
	k_sem_give(&ubx->script_running_sem);

	return 0;
}