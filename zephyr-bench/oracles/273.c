uint32_t osThreadFlagsClear(uint32_t flags)
{
	struct cmsis_rtos_thread_cb *tid;
	int sig, key;

	if (k_is_in_isr()) {
		return osFlagsErrorUnknown;
	}

	if (flags & 0x80000000) {
		return osFlagsErrorParameter;
	}

	tid = (struct cmsis_rtos_thread_cb *)osThreadGetId();
	if (tid == NULL) {
		return osFlagsErrorUnknown;
	}

	key = irq_lock();
	sig = tid->signal_results;
	tid->signal_results &= ~(flags);
	irq_unlock(key);

	return sig;
}