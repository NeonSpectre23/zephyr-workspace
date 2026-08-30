uint32_t osThreadGetCount(void)
{
	struct k_thread *thread;
	uint32_t count = 0U;

	__ASSERT(!k_is_in_isr(), "");
	for (thread = _kernel.threads; thread; thread = thread->next_thread) {
		if (get_cmsis_thread_id(thread) && z_is_thread_queued(thread)) {
			count++;
		}
	}

	return count;
}