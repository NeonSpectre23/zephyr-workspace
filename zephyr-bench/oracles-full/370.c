__boot_func
bool pm_device_is_powered(const struct device *dev)
{
#ifdef CONFIG_PM_DEVICE_POWER_DOMAIN
	struct pm_device_base *pm = dev->pm_base;

	/* If a device doesn't support PM or is not under a PM domain,
	 * assume it is always powered on.
	 */
	return (pm == NULL) ||
	       (pm->domain == NULL) ||
	       (pm->domain->pm_base->state == PM_DEVICE_STATE_ACTIVE);
#else
	ARG_UNUSED(dev);
	return true;
#endif
}