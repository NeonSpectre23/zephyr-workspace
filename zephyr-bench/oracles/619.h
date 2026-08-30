static inline int ring_buf_put_finish(struct ring_buf *buf, uint32_t size)
{
	return ring_buf_area_finish(buf, &buf->put, size);
}