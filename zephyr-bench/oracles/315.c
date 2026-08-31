bool pm_policy_state_any_active(void)
{
#if DT_HAS_COMPAT_STATUS_OKAY(zephyr_power_state)
	/* Check if there is any power state that is not locked and not disabled due
	 * to latency requirements.
	 */
	return (atomic_get(&global_lock_cnt) == 0) &&
	       (atomic_get(&unlock_mask) & atomic_get(&latency_mask));
#endif
	return true;
}