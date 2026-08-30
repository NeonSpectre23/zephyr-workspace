static inline uint32_t ring_buf_space_get(const struct ring_buf *buf)
{
	ring_buf_idx_t allocated = buf->put.head - buf->get.tail;

	return buf->size - allocated;
}