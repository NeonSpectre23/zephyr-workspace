static inline uint8_t *log_msg_get_data(struct log_msg *msg, size_t *len)
{
	*len = msg->hdr.desc.data_len;

	return msg->data + msg->hdr.desc.package_len;
}