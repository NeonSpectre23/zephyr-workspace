uint32_t osThreadEnumerate(osThreadId_t *thread_array, uint32_t array_items)
{
	struct k_thread *thread;
	uint32_t count = 0U;
	osThreadId_t tid;

	__ASSERT(!k_is_in_isr(), "");
	__ASSERT(thread_array != NULL, "");
	__ASSERT(array_items, "");

	for (thread = _kernel.threads; thread; thread = thread->next_thread) {
		if (count == array_items) {
			break;
		}

		tid = get_cmsis_thread_id(thread);
		if (tid != NULL) {
			thread_array[count] = tid;
			count++;
		}
	}

	return (count);
}