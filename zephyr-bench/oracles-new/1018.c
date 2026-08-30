uint32_t z_log_dropped_read_and_clear(void)
{
	return atomic_set(&dropped_cnt, 0);
}