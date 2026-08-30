void z_log_msg_commit(struct log_msg *msg)
{
	msg->hdr.timestamp = timestamp_func();
	msg_commit(&log_buffer, msg);
}