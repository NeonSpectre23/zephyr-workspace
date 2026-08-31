int32_t osKernelLock(void)
{
	int temp = _current->base.sched_locked;

	if (k_is_in_isr()) {
		return osErrorISR;
	}

	k_sched_lock();

	return temp;
}