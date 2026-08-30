void smp_raw_dummy_clear_state(void)
{
	k_sem_reset(&smp_data_ready_sem);

	memset(smp_receive_buffer, 0, sizeof(smp_receive_buffer));
	smp_receive_pos = 0;
	memset(smp_send_buffer, 0, sizeof(smp_send_buffer));
	smp_send_pos = 0;
}