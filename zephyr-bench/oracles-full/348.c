osThreadState_t osThreadGetState(osThreadId_t thread_id)
{
	struct cmsis_rtos_thread_cb *tid = (struct cmsis_rtos_thread_cb *)thread_id;
	osThreadState_t state;

	if (k_is_in_isr() || (tid == NULL) || (is_cmsis_rtos_v2_thread(tid) == NULL)) {
		return osThreadError;
	}

	switch (tid->z_thread.base.thread_state) {
	case _THREAD_DUMMY:
		state = osThreadError;
		break;
	case _THREAD_DEAD:
		state = osThreadTerminated;
		break;
	case _THREAD_SUSPENDED:
	case _THREAD_SLEEPING:
	case _THREAD_PENDING:
		state = osThreadBlocked;
		break;
	case _THREAD_QUEUED:
		state = osThreadReady;
		break;
	default:
		state = osThreadError;
		break;
	}

	if (osThreadGetId() == thread_id) {
		state = osThreadRunning;
	}

	return state;
}