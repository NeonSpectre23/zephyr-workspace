void z_log_msg_init(void)
{
#ifdef CONFIG_MPSC_PBUF
	mpsc_pbuf_init(&log_buffer, &mpsc_config);
	curr_log_buffer = &log_buffer;
#endif
}