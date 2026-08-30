osThreadId osThreadGetId(void)
{
	if (k_is_in_isr()) {
		return NULL;
	}

	return (osThreadId)k_current_get();
}