int pm_device_runtime_disable(const struct device *dev)
{
	int ret = 0;
	struct pm_device *pm = dev->pm;

	SYS_PORT_TRACING_FUNC_ENTER(pm, device_runtime_disable, dev);

	if (pm == NULL) {
		ret = -ENOTSUP;
		goto end;
	}

	if (!atomic_test_bit(&pm->base.flags, PM_DEVICE_FLAG_RUNTIME_ENABLED)) {
		goto end;
	}

	if (atomic_test_bit(&dev->pm_base->flags, PM_DEVICE_FLAG_ISR_SAFE)) {
		ret = runtime_disable_sync(dev);
		goto end;
	}

	if (!k_is_pre_kernel()) {
		(void)k_sem_take(&pm->lock, K_FOREVER);
	}

#ifdef CONFIG_PM_DEVICE_RUNTIME_ASYNC
	if (!k_is_pre_kernel()) {
		if ((pm->base.state == PM_DEVICE_STATE_SUSPENDING) &&
			((k_work_cancel_delayable(&pm->work) & K_WORK_RUNNING) == 0)) {
			pm->base.state = PM_DEVICE_STATE_ACTIVE;
			goto clear_bit;
		}

		/* wait until possible async suspend is completed */
		while (pm->base.state == PM_DEVICE_STATE_SUSPENDING) {
			k_event_clear(&pm->event, EVENT_MASK);
			k_sem_give(&pm->lock);

			k_event_wait(&pm->event, EVENT_MASK, false, K_FOREVER);

			(void)k_sem_take(&pm->lock, K_FOREVER);
		}
	}
#endif /* CONFIG_PM_DEVICE_RUNTIME_ASYNC */

	/* wake up the device if suspended */
	if (pm->base.state == PM_DEVICE_STATE_SUSPENDED) {
		ret = pm->base.action_cb(dev, PM_DEVICE_ACTION_RESUME);
		if (ret < 0) {
			goto unlock;
		}

		pm->base.state = PM_DEVICE_STATE_ACTIVE;
	}
#ifdef CONFIG_PM_DEVICE_RUNTIME_ASYNC
clear_bit:
#endif
	atomic_clear_bit(&pm->base.flags, PM_DEVICE_FLAG_RUNTIME_ENABLED);

unlock:
	if (!k_is_pre_kernel()) {
		k_sem_give(&pm->lock);
	}

end:
	SYS_PORT_TRACING_FUNC_EXIT(pm, device_runtime_disable, dev, ret);

	return ret;
}