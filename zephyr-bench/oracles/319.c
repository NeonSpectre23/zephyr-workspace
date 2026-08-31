void pm_policy_state_lock_get(enum pm_state state, uint8_t substate_id)
{
#if DT_HAS_COMPAT_STATUS_OKAY(zephyr_power_state)
	for (size_t i = 0; i < ARRAY_SIZE(substates); i++) {
		if (substates[i].state == state &&
		   (substates[i].substate_id == substate_id || substate_id == PM_ALL_SUBSTATES)) {
			k_spinlock_key_t key = k_spin_lock(&lock);

			if (lock_cnt[i] == 0) {
				unlock_mask &= ~BIT(i);
			}
			lock_cnt[i]++;
			k_spin_unlock(&lock, key);
		}
	}
#endif
}