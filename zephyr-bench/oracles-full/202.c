int modem_cmux_attach(struct modem_cmux *cmux, struct modem_pipe *pipe)
{
	if (cmux->pipe != NULL) {
		return -EALREADY;
	}

	cmux->pipe = pipe;
	ring_buf_reset(&cmux->transmit_rb);
	cmux->receive_state = MODEM_CMUX_RECEIVE_STATE_SOF;
	modem_pipe_attach(cmux->pipe, modem_cmux_bus_callback, cmux);

	K_SPINLOCK(&cmux->work_lock) {
		cmux->attached = true;
	}

	return 0;
}