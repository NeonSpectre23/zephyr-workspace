bool pm_device_wakeup_is_enabled(const struct device *dev)
{
	struct pm_device_base *pm = dev->pm_base;

	if (pm == NULL) {
		return false;
	}

	return atomic_test_bit(&pm->flags,
			       PM_DEVICE_FLAG_WS_ENABLED);
}