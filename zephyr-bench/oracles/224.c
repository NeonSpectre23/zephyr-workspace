osStatus osDelay(uint32_t delay_ms)
{
	if (k_is_in_isr()) {
		return osErrorISR;
	}

	k_msleep(delay_ms);
	return osEventTimeout;
}