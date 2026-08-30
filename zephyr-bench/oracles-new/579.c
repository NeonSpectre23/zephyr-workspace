osStatus osThreadYield(void)
{
	if (k_is_in_isr()) {
		return osErrorISR;
	}

	k_yield();
	return osOK;
}