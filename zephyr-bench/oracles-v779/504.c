void z_log_msg_enqueue(const struct log_link *link, const void *data, size_t len)
{
	struct log_msg *log_msg = (struct log_msg *)data;
	size_t wlen = DIV_ROUND_UP(ROUND_UP(len, Z_LOG_MSG_ALIGNMENT), sizeof(int));
	struct mpsc_pbuf_buffer *mpsc_pbuffer = link->mpsc_pbuf ? link->mpsc_pbuf : &log_buffer;
	struct log_msg *local_msg = msg_alloc(mpsc_pbuffer, wlen);

	if (!local_msg) {
		z_log_dropped(false);
		return;
	}

	log_msg->hdr.desc.valid = 0;
	log_msg->hdr.desc.busy = 0;
	log_msg->hdr.desc.domain += link->ctrl_blk->domain_offset;
	memcpy((void *)local_msg, data, len);
	msg_commit(mpsc_pbuffer, local_msg);
}