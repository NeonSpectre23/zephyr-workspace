int32_t osKernelRestoreLock(int32_t lock)
{
	_current->base.sched_locked = lock;

	if (k_is_in_isr()) {
		return osErrorISR;
	}

	if (lock < 0) {
		return 1; /* locked */
	} else {
		return 0; /* not locked */
	}
}