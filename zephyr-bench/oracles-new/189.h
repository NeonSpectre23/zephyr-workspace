static inline int ring_buf_get_finish(struct ring_buf *buf, uint32_t size)
{
	return ring_buf_area_finish(buf, &buf->get, size);
}