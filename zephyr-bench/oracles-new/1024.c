void z_log_msg_free(union log_msg_generic *msg)
{
	msg_free(curr_log_buffer, msg);
}