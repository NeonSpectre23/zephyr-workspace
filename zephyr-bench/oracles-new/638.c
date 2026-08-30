int pm_device_runtime_enable(const struct device *dev)
{
	int ret = 0;
	struct pm_device *pm = dev->pm;

	SYS_PORT_TRACING_FUNC_ENTER(pm, device_runtime_enable, dev);

	if (pm == NULL) {
		ret = -ENOTSUP;
		goto end;
	}

	if (atomic_test_bit(&pm->base.flags, PM_DEVICE_FLAG_RUNTIME_ENABLED)) {
		goto end;
	}

	if (pm_device_is_busy(dev)) {
		ret = -EBUSY;
		goto end;
	}

	if (atomic_test_bit(&dev->pm_base->flags, PM_DEVICE_FLAG_ISR_SAFE)) {
		ret = runtime_enable_sync(dev);
		goto end;
	}

	if (!k_is_pre_kernel()) {
		(void)k_sem_take(&pm->lock, K_FOREVER);
	}

	/* lazy init of PM fields */
	if (pm->dev == NULL) {
		pm->dev = dev;
#ifdef CONFIG_PM_DEVICE_RUNTIME_ASYNC
		k_work_init_delayable(&pm->work, runtime_suspend_work);
#endif /* CONFIG_PM_DEVICE_RUNTIME_ASYNC */
	}

	if (pm->base.state == PM_DEVICE_STATE_ACTIVE) {
		ret = pm->base.action_cb(pm->dev, PM_DEVICE_ACTION_SUSPEND);
		if (ret < 0) {
			goto unlock;
		}
		pm->base.state = PM_DEVICE_STATE_SUSPENDED;
	}

	pm->base.usage = 0U;

	atomic_set_bit(&pm->base.flags, PM_DEVICE_FLAG_RUNTIME_ENABLED);

unlock:
	if (!k_is_pre_kernel()) {
		k_sem_give(&pm->lock);
	}

end:
	SYS_PORT_TRACING_FUNC_EXIT(pm, device_runtime_enable, dev, ret);
	return ret;
}