void mpsc_pbuf_init(struct mpsc_pbuf_buffer *buffer,
		    const struct mpsc_pbuf_buffer_config *cfg)
{
	memset(buffer, 0, offsetof(struct mpsc_pbuf_buffer, buf));
	buffer->get_wlen = cfg->get_wlen;
	buffer->notify_drop = cfg->notify_drop;
	buffer->buf = cfg->buf;
	buffer->size = cfg->size;
	buffer->max_usage = 0;
	buffer->flags = cfg->flags;

	if (is_power_of_two(buffer->size)) {
		buffer->flags |= MPSC_PBUF_SIZE_POW2;
	}

	if (IS_ENABLED(CONFIG_MULTITHREADING)) {
		int err;

		err = k_sem_init(&buffer->sem, 0, 1);
		__ASSERT_NO_MSG(err == 0);
		ARG_UNUSED(err);
	}
}