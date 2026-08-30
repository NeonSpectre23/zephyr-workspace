int modem_ppp_attach(struct modem_ppp *ppp, struct modem_pipe *pipe)
{
	if (atomic_test_bit(&ppp->state, MODEM_PPP_STATE_ATTACHED_BIT) == true) {
		return 0;
	}

	ppp->pipe = pipe;
	modem_pipe_attach(pipe, modem_ppp_pipe_callback, ppp);

	atomic_set_bit(&ppp->state, MODEM_PPP_STATE_ATTACHED_BIT);
	return 0;
}