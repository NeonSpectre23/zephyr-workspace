struct log_msg *z_log_msg_alloc(uint32_t wlen)
{
	return msg_alloc(&log_buffer, wlen);
}