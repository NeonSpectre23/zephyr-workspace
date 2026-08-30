static inline uint8_t log_msg_get_domain(struct log_msg *msg)
{
	return msg->hdr.desc.domain;
}