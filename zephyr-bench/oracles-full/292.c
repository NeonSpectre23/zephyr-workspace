osStatus osKernelStart(void)
{
	if (k_is_in_isr()) {
		return osErrorISR;
	}
	return osOK;
}