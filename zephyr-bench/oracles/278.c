const char *osThreadGetName(osThreadId_t thread_id)
{
	struct cmsis_rtos_thread_cb *tid = (struct cmsis_rtos_thread_cb *)thread_id;

	if (tid == NULL) {
		return NULL;
	}
	return k_thread_name_get(&tid->z_thread);
}