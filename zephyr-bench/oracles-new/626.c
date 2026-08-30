void pm_device_busy_clear(const struct device *dev)
{
	struct pm_device_base *pm = dev->pm_base;

	if (pm == NULL) {
		return;
	}

	atomic_clear_bit(&pm->flags, PM_DEVICE_FLAG_BUSY);
}