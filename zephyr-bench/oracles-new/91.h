static inline const void *log_msg_get_source(struct log_msg *msg)
{
	return msg->hdr.source;
}