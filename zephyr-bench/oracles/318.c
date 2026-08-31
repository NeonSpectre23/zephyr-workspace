bool pm_policy_state_is_available(enum pm_state state, uint8_t substate_id)
{
#if DT_HAS_COMPAT_STATUS_OKAY(zephyr_power_state)
	if (atomic_get(&global_lock_cnt) != 0) {
		return false;
	}

	for (size_t i = 0; i < ARRAY_SIZE(substates); i++) {
		if (substates[i].state == state &&
		   (substates[i].substate_id == substate_id || substate_id == PM_ALL_SUBSTATES)) {
			return (atomic_get(&lock_cnt[i]) == 0) &&
			       (atomic_get(&latency_mask) & BIT(i));
		}
	}
#endif

	return false;
}