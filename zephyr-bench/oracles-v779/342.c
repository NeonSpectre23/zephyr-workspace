void pm_policy_state_all_lock_get(void)
{
#if DT_HAS_COMPAT_STATUS_OKAY(zephyr_power_state)
	(void)atomic_inc(&global_lock_cnt);
#endif
}