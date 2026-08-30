uint32_t osThreadFlagsSet(osThreadId_t thread_id, uint32_t flags)
{
	unsigned int key;
	struct cmsis_rtos_thread_cb *tid = (struct cmsis_rtos_thread_cb *)thread_id;

	if ((thread_id == NULL) || (is_cmsis_rtos_v2_thread(thread_id) == NULL) ||
	    (flags & 0x80000000)) {
		return osFlagsErrorParameter;
	}

	key = irq_lock();
	tid->signal_results |= flags;
	irq_unlock(key);

	k_poll_signal_raise(&tid->poll_signal, DONT_CARE);

	return tid->signal_results;
}