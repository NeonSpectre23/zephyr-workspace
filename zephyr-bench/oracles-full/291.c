int32_t osKernelRunning(void)
{
	return !z_is_thread_suspended(&z_main_thread);
}