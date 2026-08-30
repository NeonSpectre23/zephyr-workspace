bool pm_device_runtime_is_enabled(const struct device *dev)
{
	struct pm_device_base *pm = dev->pm_base;

	return pm && atomic_test_bit(&pm->flags, PM_DEVICE_FLAG_RUNTIME_ENABLED);
}