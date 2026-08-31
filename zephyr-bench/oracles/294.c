bool pm_device_on_power_domain(const struct device *dev)
{
#ifdef CONFIG_PM_DEVICE_POWER_DOMAIN
	struct pm_device_base *pm = dev->pm_base;

	if (pm == NULL) {
		return false;
	}
	return pm->domain != NULL;
#else
	ARG_UNUSED(dev);
	return false;
#endif
}