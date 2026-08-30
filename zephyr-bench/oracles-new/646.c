bool pm_device_wakeup_enable(const struct device *dev, bool enable)
{
	atomic_val_t flags, new_flags;
	struct pm_device_base *pm = dev->pm_base;

	if (pm == NULL) {
		return false;
	}

	flags = atomic_get(&pm->flags);

	if ((flags & BIT(PM_DEVICE_FLAG_WS_CAPABLE)) == 0U) {
		return false;
	}

	if (enable) {
		new_flags = flags |
			    BIT(PM_DEVICE_FLAG_WS_ENABLED);
	} else {
		new_flags = flags & ~BIT(PM_DEVICE_FLAG_WS_ENABLED);
	}

	return atomic_cas(&pm->flags, flags, new_flags);
}