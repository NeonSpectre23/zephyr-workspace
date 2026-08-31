static inline log_timestamp_t log_msg_get_timestamp(struct log_msg *msg)
{
	return msg->hdr.timestamp;
}