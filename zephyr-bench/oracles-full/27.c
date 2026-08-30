int cpu_freq_pstate_set(const struct pstate *state)
{
	if (state == NULL) {
		LOG_ERR("Stub pstate is NULL");
		return -EINVAL;
	}

	int state_id = ((const struct stub_config *)state->config)->state_id;

	LOG_DBG("Stub setting performance state: %d", state_id);

	switch (state_id) {
	case 0:
		LOG_DBG("Stub setting P-state 0: Nominal Mode\n");
		break;
	case 1:
		LOG_DBG("Stub setting P-state 1: Low Power Mode\n");
		break;
	case 2:
		LOG_DBG("Stub setting P-state 2: Ultra-low Power Mode\n");
		break;
	default:
		LOG_ERR("Stub unsupported P-state: %d", state_id);
		return -1;
	}

	return 0;
}