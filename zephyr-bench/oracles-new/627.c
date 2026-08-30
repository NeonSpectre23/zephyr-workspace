void pm_device_busy_set(const struct device *dev)
{
	struct pm_device_base *pm = dev->pm_base;

	if (pm == NULL) {
		return;
	}

	atomic_set_bit(&pm->flags, PM_DEVICE_FLAG_BUSY);
}