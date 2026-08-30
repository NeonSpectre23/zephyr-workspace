int pm_device_runtime_put(const struct device *dev)
{
	int ret;

	if (dev->pm_base == NULL) {
		return 0;
	}

	SYS_PORT_TRACING_FUNC_ENTER(pm, device_runtime_put, dev);

	if (atomic_test_bit(&dev->pm_base->flags, PM_DEVICE_FLAG_ISR_SAFE)) {
		struct pm_device_isr *pm_sync = dev->pm_isr;
		k_spinlock_key_t k = k_spin_lock(&pm_sync->lock);

		ret = put_sync_locked(dev);

		k_spin_unlock(&pm_sync->lock, k);
	} else {
		ret = runtime_suspend(dev, false, K_NO_WAIT);
	}
	SYS_PORT_TRACING_FUNC_EXIT(pm, device_runtime_put, dev, ret);

	return ret;
}