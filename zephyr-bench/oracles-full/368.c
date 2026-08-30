int pm_device_driver_deinit(const struct device *dev,
			    pm_device_action_cb_t action_cb)
{
	ARG_UNUSED(action_cb);

	struct pm_device_base *pm = dev->pm_base;

	return pm->state == PM_DEVICE_STATE_SUSPENDED ||
	       pm->state == PM_DEVICE_STATE_OFF ?
	       0 :
	       -EBUSY;
}