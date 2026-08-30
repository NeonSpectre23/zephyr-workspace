int cpu_freq_policy_select_pstate(const struct pstate **pstate_out)
{
	int sys_pressure = 0;
	int cpu_id = 0;

	if (NULL == pstate_out) {
		LOG_ERR("On-Demand Policy: pstate_out is NULL");
		return -EINVAL;
	}

#if defined(CONFIG_SMP)
	/* The caller has already ensured that the CPU is fixed */
	cpu_id = arch_curr_cpu()->id;
#endif

	sys_pressure = get_normalized_sys_pressure();

	if (sys_pressure < 0) {
		LOG_ERR("Unable to retrieve system pressure");
		return sys_pressure;
	}

	LOG_DBG("CPU%d Pressure: %d%%", cpu_id, sys_pressure);

	for (int i = 0; i < soc_pstates_count; i++) {
		const struct pstate *state = soc_pstates[i];

		if (sys_pressure >= state->load_threshold) {
			*pstate_out = state;
			LOG_DBG("Pressure Policy: Selected P-state "
				"%d with load_threshold=%d%%",
				i, state->load_threshold);
			return 0;
		}
	}

	/* No threshold matched: select the last P-state (lowest performance) */
	*pstate_out = soc_pstates[soc_pstates_count - 1];
	LOG_DBG("Pressure Policy: No threshold matched for CPU load %d%%;"
		"selecting last P-state (load_threshold=%d%%)",
		sys_pressure, soc_pstates[soc_pstates_count - 1]->load_threshold);

	return 0;
}