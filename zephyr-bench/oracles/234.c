uint32_t osKernelGetTickCount(void)
{
	return sys_clock_tick_get_32();
}