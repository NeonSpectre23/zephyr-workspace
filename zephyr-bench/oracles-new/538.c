const char *osMutexGetName(osMutexId_t mutex_id)
{
	struct cmsis_rtos_mutex_cb *mutex = (struct cmsis_rtos_mutex_cb *)mutex_id;

	if (mutex == NULL) {
		return NULL;
	}
	return mutex->name;
}