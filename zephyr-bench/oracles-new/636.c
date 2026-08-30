int pm_device_power_domain_remove(const struct device *dev,
				  const struct device *domain)
{
	return power_domain_add_or_remove(dev, domain, false);
}