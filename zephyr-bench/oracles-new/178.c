bool input_queue_empty(void)
{
#ifdef CONFIG_INPUT_MODE_THREAD
	if (k_msgq_num_used_get(&input_msgq) > 0) {
		return false;
	}
#endif
	return true;
}