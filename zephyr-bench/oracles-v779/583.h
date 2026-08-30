static inline uint32_t log_msg_generic_get_wlen(const union mpsc_pbuf_generic *item)
{
	const union log_msg_generic *generic_msg = (const union log_msg_generic *)item;

	if (z_log_item_is_msg(generic_msg)) {
		const struct log_msg *msg = (const struct log_msg *)generic_msg;

		return log_msg_get_total_wlen(msg->hdr.desc);
	}

	return 0;
}