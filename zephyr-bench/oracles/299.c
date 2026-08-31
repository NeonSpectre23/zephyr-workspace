int pm_device_runtime_get(const struct device *dev)
{
	int ret = 0;
	struct pm_device *pm = dev->pm;

	if (pm == NULL) {
		return 0;
	}

	SYS_PORT_TRACING_FUNC_ENTER(pm, device_runtime_get, dev);

	/*
	 * Early return if device runtime is not enabled.
	 */
	if (!atomic_test_bit(&pm->base.flags, PM_DEVICE_FLAG_RUNTIME_ENABLED)) {
		return 0;
	}

	if (atomic_test_bit(&dev->pm_base->flags, PM_DEVICE_FLAG_ISR_SAFE)) {
		struct pm_device_isr *pm_sync = dev->pm_isr;
		k_spinlock_key_t k = k_spin_lock(&pm_sync->lock);

		ret = get_sync_locked(dev);
		k_spin_unlock(&pm_sync->lock, k);
		goto end;
	}

	if (!k_is_pre_kernel()) {
		ret = k_sem_take(&pm->lock, k_is_in_isr() ? K_NO_WAIT : K_FOREVER);
		if (ret < 0) {
			return -EWOULDBLOCK;
		}
	}

	if (k_is_in_isr() && (pm->base.state == PM_DEVICE_STATE_SUSPENDING)) {
		ret = -EWOULDBLOCK;
		goto unlock;
	}

	/*
	 * If the device is under a power domain, the domain has to be get
	 * first.
	 */
	const struct device *domain = PM_DOMAIN(&pm->base);

	if (domain != NULL && !atomic_test_bit(&dev->pm_base->flags, PM_DEVICE_FLAG_PD_CLAIMED)) {
		ret = pm_device_runtime_get(domain);
		if (ret != 0) {
			goto unlock;
		}
		/* Check if powering up this device failed */
		if (atomic_test_bit(&pm->base.flags, PM_DEVICE_FLAG_TURN_ON_FAILED)) {
			(void)pm_device_runtime_put(domain);
			ret = -EAGAIN;
			goto unlock;
		}
		/* Power domain successfully claimed */
		atomic_set_bit(&pm->base.flags, PM_DEVICE_FLAG_PD_CLAIMED);
	}

	pm->base.usage++;

#ifdef CONFIG_PM_DEVICE_RUNTIME_ASYNC
	/*
	 * Check if the device has a pending suspend operation (not started
	 * yet) and cancel it. This way we avoid unnecessary operations because
	 * the device is actually active.
	 */
	if ((pm->base.state == PM_DEVICE_STATE_SUSPENDING) &&
		((k_work_cancel_delayable(&pm->work) & K_WORK_RUNNING) == 0)) {
		pm->base.state = PM_DEVICE_STATE_ACTIVE;
		goto unlock;
	}

	if (!k_is_pre_kernel()) {
		/*
		 * If the device is already suspending there is
		 * nothing else we can do but wait until it finishes.
		 */
		while (pm->base.state == PM_DEVICE_STATE_SUSPENDING) {
			k_event_clear(&pm->event, EVENT_MASK);
			k_sem_give(&pm->lock);

			k_event_wait(&pm->event, EVENT_MASK, false, K_FOREVER);

			(void)k_sem_take(&pm->lock, K_FOREVER);
		}
	}
#endif /* CONFIG_PM_DEVICE_RUNTIME_ASYNC */

	if (pm->base.usage > 1U) {
		goto unlock;
	}

	ret = pm->base.action_cb(pm->dev, PM_DEVICE_ACTION_RESUME);
	if (ret < 0) {
		pm->base.usage--;
		if (domain != NULL) {
			(void)pm_device_runtime_put(domain);
			atomic_clear_bit(&dev->pm_base->flags, PM_DEVICE_FLAG_PD_CLAIMED);
		}
		goto unlock;
	}

	pm->base.state = PM_DEVICE_STATE_ACTIVE;

unlock:
	if (!k_is_pre_kernel()) {
		k_sem_give(&pm->lock);
	}

end:
	SYS_PORT_TRACING_FUNC_EXIT(pm, device_runtime_get, dev, ret);

	return ret;
}