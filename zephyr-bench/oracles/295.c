int pm_device_power_domain_add(const struct device *dev,
			       const struct device *domain)
{
	return power_domain_add_or_remove(dev, domain, true);
}