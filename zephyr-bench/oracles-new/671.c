bool pm_state_force(uint8_t cpu, const struct pm_state_info *info)
{
	k_spinlock_key_t key;

	__ASSERT(info->state < PM_STATE_COUNT,
		 "Invalid power state %d!", info->state);

	info = pm_state_get(cpu, info->state, info->substate_id);
	if (info == NULL) {
		/* Return false if the state could not be retrieved */
		return false;
	}

	key = k_spin_lock(&pm_forced_state_lock);
	z_cpus_pm_forced_state[cpu] = info;
	k_spin_unlock(&pm_forced_state_lock, key);

	return true;
}