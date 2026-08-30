__boot_func
int pm_device_driver_init(const struct device *dev,
			  pm_device_action_cb_t action_cb)
{
	struct pm_device_base *pm = dev->pm_base;
	int rc;

	/* Device is currently in the OFF state */
	if (pm) {
		pm->state = PM_DEVICE_STATE_OFF;
	}

	/* Work only needs to be performed if the device is powered */
	if (!pm_device_is_powered(dev)) {
		return 0;
	}

	/* Run power-up logic */
	rc = action_cb(dev, PM_DEVICE_ACTION_TURN_ON);
	if ((rc < 0) && (rc != -ENOTSUP)) {
		return rc;
	}

	/* If device has no PM structure */
	if (pm == NULL) {
		/* Device should always be active */
		return action_cb(dev, PM_DEVICE_ACTION_RESUME);
	}

	/* Device is currently in the SUSPENDED state */
	pm->state = PM_DEVICE_STATE_SUSPENDED;

	/* If device will have PM device runtime enabled */
	if (IS_ENABLED(CONFIG_PM_DEVICE_RUNTIME) &&
	    (IS_ENABLED(CONFIG_PM_DEVICE_RUNTIME_DEFAULT_ENABLE) ||
	     atomic_test_bit(&pm->flags, PM_DEVICE_FLAG_RUNTIME_AUTO))) {
		return 0;
	}

	/* Startup into active mode */
	rc = action_cb(dev, PM_DEVICE_ACTION_RESUME);
	if (rc < 0) {
		return rc;
	}

	/* Device is now in the ACTIVE state */
	pm->state = PM_DEVICE_STATE_ACTIVE;

	return 0;
}