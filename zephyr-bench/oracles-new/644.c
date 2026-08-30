int pm_device_state_get(const struct device *dev,
			enum pm_device_state *state)
{
	struct pm_device_base *pm = dev->pm_base;

	if (pm == NULL) {
		return -ENOSYS;
	}

	*state = pm->state;

	return 0;
}