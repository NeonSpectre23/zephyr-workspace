static inline bool ring_buf_is_empty(const struct ring_buf *buf)
{
	return buf->get.head == buf->put.tail;
}