int pm_device_action_run(const struct device *dev,
			 enum pm_device_action action)
{
	struct pm_device_base *pm = dev->pm_base;
	int ret;

	if (pm == NULL) {
		return -ENOSYS;
	}

	/* Validate action against current state */
	if (pm->state == action_target_state[action]) {
		return -EALREADY;
	}
	if (pm->state != action_expected_state[action]) {
		return -ENOTSUP;
	}

	ret = pm->action_cb(dev, action);
	if (ret < 0) {
		/*
		 * TURN_ON and TURN_OFF are actions triggered by a power domain
		 * when it is resumed or suspended, which means that the energy
		 * to the device will be removed or added. For this reason, if
		 * the transition fails or the device does not handle these
		 * actions its state still needs to updated to reflect its
		 * physical behavior.
		 *
		 * The function will still return the error code so the domain
		 * can take whatever action is more appropriated.
		 */
		switch (action) {
		case PM_DEVICE_ACTION_TURN_ON:
			/* Store an error flag when the transition explicitly fails */
			if (ret != -ENOTSUP) {
				atomic_set_bit(&pm->flags, PM_DEVICE_FLAG_TURN_ON_FAILED);
			}
			__fallthrough;
		case PM_DEVICE_ACTION_TURN_OFF:
			pm->state = action_target_state[action];
			break;
		default:
			break;
		}
		return ret;
	}

	pm->state = action_target_state[action];
	/* Power up flags are no longer relevant */
	if (action == PM_DEVICE_ACTION_TURN_OFF) {
		atomic_clear_bit(&pm->flags, PM_DEVICE_FLAG_PD_CLAIMED);
		atomic_clear_bit(&pm->flags, PM_DEVICE_FLAG_TURN_ON_FAILED);
	}

	return 0;
}