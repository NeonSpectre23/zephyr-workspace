bool smp_dummy_wait_for_data(uint32_t wait_time_s)
{
	return k_sem_take(&smp_data_ready_sem, K_SECONDS(wait_time_s)) == 0 ? true : false;
}