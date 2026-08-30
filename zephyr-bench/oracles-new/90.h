static inline uint8_t *log_msg_get_package(struct log_msg *msg, size_t *len)
{
	*len = msg->hdr.desc.package_len;

	return msg->data;
}