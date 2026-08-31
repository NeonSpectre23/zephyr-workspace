osStatus_t osDelayUntil(uint32_t ticks)
{
	uint32_t ticks_elapsed;

	if (k_is_in_isr()) {
		return osErrorISR;
	}

	ticks_elapsed = osKernelGetTickCount();
	k_sleep(K_TICKS(ticks - ticks_elapsed));

	return osOK;
}