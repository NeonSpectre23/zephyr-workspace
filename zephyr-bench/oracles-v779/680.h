static inline uint32_t ring_buf_size_get(const struct ring_buf *buf)
{
	ring_buf_idx_t available = buf->put.tail - buf->get.head;

	return available;
}