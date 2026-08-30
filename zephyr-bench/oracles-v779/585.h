static inline uint8_t log_msg_get_level(struct log_msg *msg)
{
	return msg->hdr.desc.level;
}