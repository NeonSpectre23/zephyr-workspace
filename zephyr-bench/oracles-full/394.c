void pm_policy_state_all_lock_put(void)
{
#if DT_HAS_COMPAT_STATUS_OKAY(zephyr_power_state)
	__ASSERT(global_lock_cnt > 0, "Unbalanced state lock get/put");
	(void)atomic_dec(&global_lock_cnt);
#endif
}