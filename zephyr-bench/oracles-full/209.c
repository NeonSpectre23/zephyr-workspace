void modem_cmux_release(struct modem_cmux *cmux)
{
	struct k_work_sync sync;

	if (cmux->pipe == NULL) {
		return;
	}

	K_SPINLOCK(&cmux->work_lock) {
		cmux->attached = false;
	}

	/* Close DLCI pipes and cancel DLCI work */
	modem_cmux_dlci_pipes_release(cmux);

	/* Release bus pipe */
	if (cmux->pipe) {
		modem_pipe_release(cmux->pipe);
	}

	/* Cancel all work */
	k_work_cancel_delayable_sync(&cmux->connect_work, &sync);
	k_work_cancel_delayable_sync(&cmux->disconnect_work, &sync);
	k_work_cancel_delayable_sync(&cmux->transmit_work, &sync);
	k_work_cancel_delayable_sync(&cmux->receive_work, &sync);

	/* Unreference pipe */
	cmux->pipe = NULL;

	/* Reset state */
	set_state(cmux, MODEM_CMUX_STATE_DISCONNECTED);
}