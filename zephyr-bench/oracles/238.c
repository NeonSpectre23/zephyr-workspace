int32_t osKernelUnlock(void)
{
	int temp = _current->base.sched_locked;

	if (k_is_in_isr()) {
		return osErrorISR;
	}

	k_sched_unlock();

	return temp;
}