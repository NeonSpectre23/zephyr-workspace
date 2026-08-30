int32_t osSignalClear(osThreadId thread_id, int32_t signals)
{
	int sig, key;

	if (k_is_in_isr() || (thread_id == NULL) || (!signals) ||
		(signals & 0x80000000) || (signals > MAX_VALID_SIGNAL_VAL)) {
		return 0x80000000;
	}

	osThreadDef_t *thread_def =
		(osThreadDef_t *)k_thread_other_custom_data_get(
						(struct k_thread *)thread_id);

	key = irq_lock();
	sig = thread_def->signal_results;
	thread_def->signal_results &= ~(signals);
	irq_unlock(key);

	return sig;
}