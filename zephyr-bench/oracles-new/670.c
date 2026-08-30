uint8_t pm_state_cpu_get_all(uint8_t cpu, const struct pm_state_info **states)
{
	if (cpu >= ARRAY_SIZE(cpus_states)) {
		return 0;
	}

	*states = cpus_states[cpu];

	return states_per_cpu[cpu];
}