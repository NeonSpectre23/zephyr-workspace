uint32_t osKernelSysTick(void)
{
	return k_cycle_get_32();
}