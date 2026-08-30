static inline void ring_buf_internal_reset(struct ring_buf *buf, ring_buf_idx_t value)
{
	buf->put.head = buf->put.tail = buf->put.base = value;
	buf->get.head = buf->get.tail = buf->get.base = value;
}