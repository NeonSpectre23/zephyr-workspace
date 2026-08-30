int pm_device_runtime_usage(const struct device *dev)
{
	if (!pm_device_runtime_is_enabled(dev)) {
		return -ENOTSUP;
	}

	return dev->pm_base->usage;
}